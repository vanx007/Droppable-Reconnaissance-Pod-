#ifndef DRP_DHT22_DRIVER_H
#define DRP_DHT22_DRIVER_H

#include <Arduino.h>
#include <DHT.h>

struct ClimateData {
    float temperatureC;
    float humidityPct;
    float heatIndexC;
    bool isValid;
};

class ClimateSensorDriver {
public:
    ClimateSensorDriver(uint8_t pin = 4, uint8_t type = DHT22) : _dht(pin, type) {
        memset(&_data, 0, sizeof(ClimateData));
        _data.temperatureC = 25.0f;
        _data.humidityPct = 50.0f;
        _data.isValid = false;
    }

    void begin() {
        _dht.begin();
    }

    bool read() {
        float h = _dht.readHumidity();
        float t = _dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            _data.isValid = false;
            return false;
        }

        _data.temperatureC = t;
        _data.humidityPct = h;
        _data.heatIndexC = _dht.computeHeatIndex(t, h, false);
        _data.isValid = true;
        return true;
    }

    ClimateData getData() const { return _data; }

private:
    DHT _dht;
    ClimateData _data;
};

#endif // DRP_DHT22_DRIVER_H
