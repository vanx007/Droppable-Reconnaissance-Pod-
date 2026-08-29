#include "sensors/mhz19_driver.h"

MHZ19Driver::MHZ19Driver() 
    : _serial(nullptr), _lastReadMs(0), _prevPpm(420), _historyIndex(0), _historyFilled(false) {
    memset(&_data, 0, sizeof(CO2Data));
    _data.co2_ppm = 420;
    _data.isWarmedUp = false;
    _data.isValid = false;
    for (int i = 0; i < 10; i++) _historyPpm[i] = 420.0f;
}

void MHZ19Driver::begin(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin) {
    _serial = serialPort;
    _serial->begin(9600, SERIAL_8N1, rxPin, txPin);
}

uint8_t MHZ19Driver::calculateChecksum(const uint8_t* packet) {
    uint8_t checksum = 0;
    for (uint8_t i = 1; i < 8; i++) {
        checksum += packet[i];
    }
    checksum = 0xFF - checksum;
    checksum += 1;
    return checksum;
}

bool MHZ19Driver::read() {
    if (!_serial) return false;

    // Command to read CO2 concentration: 0xFF 0x01 0x86 0x00 0x00 0x00 0x00 0x00 0x79
    uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    
    // Clear buffer
    while (_serial->available()) _serial->read();

    _serial->write(cmd, 9);
    
    unsigned long startWait = millis();
    while (_serial->available() < 9) {
        if (millis() - startWait > 120) {
            return false; // Timeout
        }
        delay(2);
    }

    uint8_t response[9];
    _serial->readBytes(response, 9);

    if (response[0] == 0xFF && response[1] == 0x86) {
        uint8_t calcCrc = calculateChecksum(response);
        if (calcCrc == response[8]) {
            uint16_t ppm = (response[2] << 8) | response[3];
            int8_t temp = response[4] - 40;

            unsigned long now = millis();
            float dt_sec = (_lastReadMs > 0) ? (float)(now - _lastReadMs) / 1000.0f : 1.0f;
            if (dt_sec < 0.1f) dt_sec = 1.0f;

            _data.co2_ppm = ppm;
            _data.temp_c = temp;
            _data.isValid = true;

            // Warmup is typically 60-120 seconds
            if (millis() > 60000) _data.isWarmedUp = true;

            // Rolling rate computation (ppm/sec)
            _historyPpm[_historyIndex] = (float)ppm;
            _historyIndex = (_historyIndex + 1) % 10;
            if (_historyIndex == 0) _historyFilled = true;

            float oldest = _historyFilled ? _historyPpm[_historyIndex] : _historyPpm[0];
            float newest = (float)ppm;
            float totalDt = _historyFilled ? 10.0f : (_historyIndex * 1.0f);
            if (totalDt < 1.0f) totalDt = 1.0f;

            _data.co2_rate_ppm_s = (newest - oldest) / totalDt;
            _lastReadMs = now;
            return true;
        }
    }
    return false;
}

CO2Data MHZ19Driver::getData() const {
    return _data;
}

float MHZ19Driver::getNormalizedCO2Score() const {
    if (!_data.isValid) return 0.0f;

    // Score based on elevated CO2 (>500ppm) and positive gradient
    float ppmScore = 0.0f;
    if (_data.co2_ppm > 450) {
        ppmScore = ((float)(_data.co2_ppm - 450) / 400.0f) * 50.0f; // 450-850 ppm range
        if (ppmScore > 50.0f) ppmScore = 50.0f;
    }

    float rateScore = 0.0f;
    if (_data.co2_rate_ppm_s > 0.2f) {
        rateScore = (_data.co2_rate_ppm_s / 2.0f) * 50.0f;
        if (rateScore > 50.0f) rateScore = 50.0f;
    }

    return fmin(100.0f, ppmScore + rateScore);
}
