#include "sensors/mpu6050_driver.h"
#include <math.h>

MPU6050Driver::MPU6050Driver() : _lastUpdateMs(0) {
    memset(&_data, 0, sizeof(IMUData));
    _data.orientationWeight = 1.0f;
}

bool MPU6050Driver::begin(int sdaPin, int sclPin) {
    Wire.begin(sdaPin, sclPin, 400000); // 400kHz Fast I2C

    // Wake up MPU6050 (write 0 to PWR_MGMT_1 register 0x6B)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false; // MPU6050 not acknowledging
    }

    // Configure Accelerometer to +/- 8g range (0x1C = 0x10)
    writeRegister(0x1C, 0x10);

    // Configure Gyroscope to +/- 500 deg/s (0x1B = 0x08)
    writeRegister(0x1B, 0x08);

    // Configure DLPF (Digital Low Pass Filter) to 44Hz (0x1A = 0x03)
    writeRegister(0x1A, 0x03);

    return true;
}

void MPU6050Driver::writeRegister(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void MPU6050Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t count) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)MPU_ADDR, count);
    for (uint8_t i = 0; i < count && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
}

bool MPU6050Driver::update() {
    uint8_t rawBuffer[14];
    readRegisters(0x3B, rawBuffer, 14);

    int16_t rawAx = (rawBuffer[0] << 8) | rawBuffer[1];
    int16_t rawAy = (rawBuffer[2] << 8) | rawBuffer[3];
    int16_t rawAz = (rawBuffer[4] << 8) | rawBuffer[5];
    int16_t rawGx = (rawBuffer[8] << 8) | rawBuffer[9];
    int16_t rawGy = (rawBuffer[10] << 8) | rawBuffer[11];
    int16_t rawGz = (rawBuffer[12] << 8) | rawBuffer[13];

    // Scale Accelerometer (+/- 8g -> 4096 LSB/g)
    _data.accelX_g = (float)rawAx / 4096.0f;
    _data.accelY_g = (float)rawAy / 4096.0f;
    _data.accelZ_g = (float)rawAz / 4096.0f;

    // Scale Gyroscope (+/- 500 deg/s -> 65.5 LSB/deg/s)
    _data.gyroX_dps = (float)rawGx / 65.5f;
    _data.gyroY_dps = (float)rawGy / 65.5f;
    _data.gyroZ_dps = (float)rawGz / 65.5f;

    // Compute Pitch & Roll (in degrees)
    _data.pitchDeg = atan2(_data.accelY_g, sqrt(_data.accelX_g * _data.accelX_g + _data.accelZ_g * _data.accelZ_g)) * 180.0f / M_PI;
    _data.rollDeg  = atan2(-_data.accelX_g, _data.accelZ_g) * 180.0f / M_PI;

    // Compute total tilt angle deviation from vertical gravity vector
    float totalAccelMag = sqrt(_data.accelX_g * _data.accelX_g + _data.accelY_g * _data.accelY_g + _data.accelZ_g * _data.accelZ_g);
    if (totalAccelMag > 0.01f) {
        float cosTilt = fabs(_data.accelZ_g) / totalAccelMag;
        if (cosTilt > 1.0f) cosTilt = 1.0f;
        _data.tiltAngleDeg = acos(cosTilt) * 180.0f / M_PI;
        
        // Orientation weight W_theta = cos(tilt) with clamp [0.15, 1.0]
        _data.orientationWeight = fmax(0.15f, cosTilt);
    }

    // Impact shock detection (drop threshold > 3.5g magnitude)
    if (totalAccelMag > 3.5f) {
        _data.impactShockDetected = true;
    } else {
        _data.impactShockDetected = false;
    }

    return true;
}

IMUData MPU6050Driver::getData() const {
    return _data;
}

float MPU6050Driver::getTiltAngle() const {
    return _data.tiltAngleDeg;
}

float MPU6050Driver::getOrientationWeight() const {
    return _data.orientationWeight;
}
