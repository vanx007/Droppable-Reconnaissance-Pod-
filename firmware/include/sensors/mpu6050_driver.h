#ifndef DRP_MPU6050_DRIVER_H
#define DRP_MPU6050_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

struct IMUData {
    float accelX_g;
    float accelY_g;
    float accelZ_g;
    float gyroX_dps;
    float gyroY_dps;
    float gyroZ_dps;
    float pitchDeg;
    float rollDeg;
    float tiltAngleDeg;  // Deviation angle from upright bore-sight horizon
    float orientationWeight; // W_theta = cos(tilt)
    bool impactShockDetected;
};

class MPU6050Driver {
public:
    MPU6050Driver();
    bool begin(int sdaPin = 21, int sclPin = 22);
    bool update();
    IMUData getData() const;
    float getTiltAngle() const;
    float getOrientationWeight() const;

private:
    const uint8_t MPU_ADDR = 0x68;
    IMUData _data;
    unsigned long _lastUpdateMs;

    void writeRegister(uint8_t reg, uint8_t data);
    void readRegisters(uint8_t reg, uint8_t* buffer, uint8_t count);
};

#endif // DRP_MPU6050_DRIVER_H
