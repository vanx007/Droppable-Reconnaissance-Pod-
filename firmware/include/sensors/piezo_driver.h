#ifndef DRP_PIEZO_DRIVER_H
#define DRP_PIEZO_DRIVER_H

#include <Arduino.h>

struct AcousticData {
    uint16_t peakRawAdc;
    float peakVoltageMv;
    uint8_t knockEventCount;
    float normalizedAcousticScore;
};

class PiezoDriver {
public:
    PiezoDriver(uint8_t adcPin = 34) 
        : _pin(adcPin), _noiseFloor(150), _peakThreshold(1200), _peakHold(0), _knockCount(0) {
        memset(&_data, 0, sizeof(AcousticData));
    }

    void begin() {
        pinMode(_pin, INPUT);
        analogReadResolution(12); // 12-bit ADC (0-4095)
    }

    void sample() {
        uint16_t val = analogRead(_pin);
        if (val > _peakHold) {
            _peakHold = val;
        }

        if (val > _peakThreshold) {
            _knockCount++;
        }
    }

    AcousticData update(float decayFactor = 0.85f) {
        _data.peakRawAdc = _peakHold;
        _data.peakVoltageMv = ((float)_peakHold / 4095.0f) * 3300.0f;
        _data.knockEventCount = _knockCount;

        if (_peakHold > _noiseFloor) {
            float excess = (float)(_peakHold - _noiseFloor);
            _data.normalizedAcousticScore = fmin(100.0f, (excess / (float)(_peakThreshold - _noiseFloor)) * 100.0f);
        } else {
            _data.normalizedAcousticScore = 0.0f;
        }

        // Decay peak and reset knock counter for next window
        _peakHold = (uint16_t)((float)_peakHold * decayFactor);
        _knockCount = 0;

        return _data;
    }

    AcousticData getData() const { return _data; }
    float getScore() const { return _data.normalizedAcousticScore; }

private:
    uint8_t _pin;
    uint16_t _noiseFloor;
    uint16_t _peakThreshold;
    uint16_t _peakHold;
    uint8_t _knockCount;
    AcousticData _data;
};

#endif // DRP_PIEZO_DRIVER_H
