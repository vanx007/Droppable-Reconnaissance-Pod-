#include "telemetry/comm_manager.h"
#include <WiFi.h>

CommManager::CommManager() : _lastBroadcastMs(0) {}

void CommManager::begin() {
    // Start Wi-Fi Access Point for standalone local field operations
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DRP_AP_SSID, DRP_AP_PASS);

    Serial.println(F("[COMM] DRP Standalone Access Point Started"));
    Serial.print(F("[COMM] SSID: "));
    Serial.println(DRP_AP_SSID);
    Serial.print(F("[COMM] IP:   "));
    Serial.println(WiFi.softAPIP());
}

void CommManager::update() {
    // Communication loop housekeeping
}

String CommManager::buildJsonPayload(
    const RadarData& radar,
    bool dopplerTrigger,
    const IMUData& imu,
    const CO2Data& co2,
    const ClimateData& climate,
    const AcousticData& acoustic,
    const FusionTelemetry& fusion,
    float batteryVolts
) {
    StaticJsonDocument<768> doc;

    doc["device_id"] = POD_DEVICE_ID;
    doc["fw"] = FIRMWARE_VERSION;
    doc["uptime_ms"] = millis();
    doc["vbat"] = batteryVolts;

    // Radar Subsystem
    JsonObject rObj = doc.createNestedObject("radar");
    rObj["has_target"] = radar.hasTarget;
    rObj["is_static"] = radar.isStatic;
    rObj["is_moving"] = radar.isMoving;
    rObj["dist_cm"] = radar.hasTarget ? (radar.isStatic ? radar.staticDistanceCm : radar.movingDistanceCm) : 0;
    rObj["static_energy"] = radar.staticEnergy;
    rObj["moving_energy"] = radar.movingEnergy;
    rObj["raw_score"] = fusion.radarScore;

    // Doppler Trigger Subsystem
    doc["doppler_wake"] = dopplerTrigger;

    // IMU & Orientation Subsystem
    JsonObject imuObj = doc.createNestedObject("imu");
    imuObj["pitch"] = imu.pitchDeg;
    imuObj["roll"] = imu.rollDeg;
    imuObj["tilt_deg"] = imu.tiltAngleDeg;
    imuObj["w_theta"] = imu.orientationWeight;
    imuObj["impact_shock"] = imu.impactShockDetected;

    // Environmental Subsystem
    JsonObject envObj = doc.createNestedObject("environment");
    envObj["co2_ppm"] = co2.co2_ppm;
    envObj["co2_rate"] = co2.co2_rate_ppm_s;
    envObj["temp_c"] = climate.temperatureC;
    envObj["humidity_pct"] = climate.humidityPct;

    // Acoustic Subsystem
    JsonObject acObj = doc.createNestedObject("acoustic");
    acObj["peak_mv"] = acoustic.peakVoltageMv;
    acObj["knocks"] = acoustic.knockEventCount;
    acObj["score"] = acoustic.normalizedAcousticScore;

    // Fused Output
    JsonObject fObj = doc.createNestedObject("fusion");
    fObj["confidence_pct"] = fusion.finalConfidence;
    fObj["state_code"] = (int)fusion.classification;
    fObj["classification"] = fusion.classificationText;

    String output;
    serializeJson(doc, output);
    return output;
}

void CommManager::broadcast(const String& jsonPayload) {
    // 1. Output over Hardware USB Serial (for data logger / direct laptop bridge)
    Serial.println(jsonPayload);

    // 2. Future: Broadcast over ESP Async WebSocket / ESP-NOW mesh
}
