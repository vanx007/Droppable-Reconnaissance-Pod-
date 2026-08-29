#ifndef DRP_COMM_MANAGER_H
#define DRP_COMM_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors/ld2410_driver.h"
#include "sensors/mpu6050_driver.h"
#include "sensors/mhz19_driver.h"
#include "sensors/dht22_driver.h"
#include "sensors/piezo_driver.h"
#include "fusion/confidence_engine.h"

class CommManager {
public:
    CommManager();
    void begin();
    void update();
    
    String buildJsonPayload(
        const RadarData& radar,
        bool dopplerTrigger,
        const IMUData& imu,
        const CO2Data& co2,
        const ClimateData& climate,
        const AcousticData& acoustic,
        const FusionTelemetry& fusion,
        float batteryVolts
    );

    void broadcast(const String& jsonPayload);

private:
    unsigned long _lastBroadcastMs;
};

#endif // DRP_COMM_MANAGER_H
