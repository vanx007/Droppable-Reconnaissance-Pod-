#include "sensors/ld2410_driver.h"

LD2410Driver::LD2410Driver() : _serial(nullptr), _bufferIndex(0) {
    memset(&_data, 0, sizeof(RadarData));
}

void LD2410Driver::begin(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin, uint32_t baudRate) {
    _serial = serialPort;
    _serial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
}

bool LD2410Driver::readFrame() {
    if (!_serial) return false;

    while (_serial->available()) {
        uint8_t byteIn = _serial->read();

        if (_bufferIndex == 0) {
            if (byteIn == 0xF4) {
                _frameBuffer[_bufferIndex++] = byteIn;
            }
        } else if (_bufferIndex == 1) {
            if (byteIn == 0xF3) {
                _frameBuffer[_bufferIndex++] = byteIn;
            } else {
                _bufferIndex = 0;
            }
        } else if (_bufferIndex == 2) {
            if (byteIn == 0xF2) {
                _frameBuffer[_bufferIndex++] = byteIn;
            } else {
                _bufferIndex = 0;
            }
        } else if (_bufferIndex == 3) {
            if (byteIn == 0xF1) {
                _frameBuffer[_bufferIndex++] = byteIn;
            } else {
                _bufferIndex = 0;
            }
        } else {
            _frameBuffer[_bufferIndex++] = byteIn;

            // Standard basic reporting frame is typically 23 bytes (F4 F3 F2 F1 ... F8 F7 F6 F5)
            if (_bufferIndex >= 23 && _frameBuffer[_bufferIndex - 4] == 0xF8 &&
                _frameBuffer[_bufferIndex - 3] == 0xF7 &&
                _frameBuffer[_bufferIndex - 2] == 0xF6 &&
                _frameBuffer[_bufferIndex - 1] == 0xF5) {

                bool parsed = parseStandardFrame(_frameBuffer, _bufferIndex);
                _bufferIndex = 0;
                return parsed;
            }

            if (_bufferIndex >= 64) {
                _bufferIndex = 0; // Buffer overflow guard
            }
        }
    }
    return false;
}

bool LD2410Driver::parseStandardFrame(const uint8_t* buffer, uint8_t len) {
    // Frame format:
    // Header: F4 F3 F2 F1 (4 bytes)
    // Intra-frame length: 2 bytes (bytes 4-5)
    // Data type: 1 byte (byte 6, 0x02 = target state)
    // Head: 1 byte (byte 7, 0xAA)
    // Target state: 1 byte (byte 8: 0x00=No target, 0x01=Moving, 0x02=Static, 0x03=Both)
    // Moving target distance: 2 bytes (bytes 9-10)
    // Moving target energy: 1 byte (byte 11)
    // Static target distance: 2 bytes (bytes 12-13)
    // Static target energy: 1 byte (byte 14)
    // Detection distance: 2 bytes (bytes 15-16)

    if (len < 23) return false;

    uint8_t targetState = buffer[8];
    _data.hasTarget = (targetState != 0x00);
    _data.isMoving  = (targetState == 0x01 || targetState == 0x03);
    _data.isStatic  = (targetState == 0x02 || targetState == 0x03);

    _data.movingDistanceCm = buffer[9] | (buffer[10] << 8);
    _data.movingEnergy     = buffer[11];
    _data.staticDistanceCm = buffer[12] | (buffer[13] << 8);
    _data.staticEnergy     = buffer[14];
    _data.detectionDistanceCm = buffer[15] | (buffer[16] << 8);

    return true;
}

RadarData LD2410Driver::getData() const {
    return _data;
}

float LD2410Driver::getNormalizedPresenceScore() const {
    if (!_data.hasTarget) return 0.0f;

    // Weighted composite of static (micro-motion) and moving energy
    float staticScore = (float)_data.staticEnergy;
    float movingScore = (float)_data.movingEnergy;

    float composite = (staticScore * 0.65f) + (movingScore * 0.35f);
    if (composite > 100.0f) composite = 100.0f;
    return composite;
}
