#ifndef DRP_LD2410_DRIVER_H
#define DRP_LD2410_DRIVER_H

#include <Arduino.h>

struct RadarData {
    bool hasTarget;
    bool isMoving;
    bool isStatic;
    uint16_t movingDistanceCm;
    uint8_t movingEnergy;
    uint16_t staticDistanceCm;
    uint8_t staticEnergy;
    uint16_t detectionDistanceCm;
    uint8_t gateEnergies[9]; // Engineering mode per-gate energy
};

class LD2410Driver {
public:
    LD2410Driver();
    void begin(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin, uint32_t baudRate = 256000);
    bool readFrame();
    RadarData getData() const;
    float getNormalizedPresenceScore() const;

private:
    HardwareSerial* _serial;
    RadarData _data;
    uint8_t _frameBuffer[64];
    uint8_t _bufferIndex;

    bool parseStandardFrame(const uint8_t* buffer, uint8_t len);
};

#endif // DRP_LD2410_DRIVER_H
