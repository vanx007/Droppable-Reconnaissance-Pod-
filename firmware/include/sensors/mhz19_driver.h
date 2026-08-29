#ifndef DRP_MHZ19_DRIVER_H
#define DRP_MHZ19_DRIVER_H

#include <Arduino.h>

struct CO2Data {
    uint16_t co2_ppm;
    int8_t temp_c;
    float co2_rate_ppm_s;  // Rate of change in ppm/sec
    bool isWarmedUp;
    bool isValid;
};

class MHZ19Driver {
public:
    MHZ19Driver();
    void begin(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin);
    bool read();
    CO2Data getData() const;
    float getNormalizedCO2Score() const;

private:
    HardwareSerial* _serial;
    CO2Data _data;
    unsigned long _lastReadMs;
    uint16_t _prevPpm;
    float _historyPpm[10];
    uint8_t _historyIndex;
    bool _historyFilled;

    uint8_t calculateChecksum(const uint8_t* packet);
};

#endif // DRP_MHZ19_DRIVER_H
