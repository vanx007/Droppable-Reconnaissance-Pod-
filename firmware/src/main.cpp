#include <Arduino.h>
#include "config.h"
#include "sensors/ld2410_driver.h"
#include "sensors/mpu6050_driver.h"
#include "sensors/mhz19_driver.h"
#include "sensors/dht22_driver.h"
#include "sensors/piezo_driver.h"
#include "fusion/confidence_engine.h"
#include "telemetry/comm_manager.h"

// Hardware Serial Ports on ESP32
HardwareSerial SerialRadar(2); // UART2: GPIO 16 (RX), GPIO 17 (TX)
HardwareSerial SerialMHZ(1);   // UART1: GPIO 19 (RX), GPIO 18 (TX)

// Sensor Instances
LD2410Driver         radarSensor;
MPU6050Driver        imuSensor;
MHZ19Driver          co2Sensor;
ClimateSensorDriver  climateSensor(PIN_DHT22_DATA);
PiezoDriver          piezoSensor(PIN_PIEZO_ADC);
ConfidenceEngine     fusionEngine;
CommManager          commManager;

// Volatile Flags for Hardware Interrupts
volatile bool rcwlTriggerActive = false;
unsigned long lastRcwlTriggerMs = 0;

void IRAM_ATTR isrRcwlMotion() {
    rcwlTriggerActive = true;
}

// FreeRTOS Task Handles
TaskHandle_t hRadarDspTask;
TaskHandle_t hEnvFusionTask;

// FreeRTOS Task: Core 0 - High-Rate Radar & Acoustic DSP (50 Hz)
void vRadarDspTask(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms = 50Hz

    for (;;) {
        // Sample fast acoustic ADC
        piezoSensor.sample();

        // Read incoming 24GHz radar frames
        radarSensor.readFrame();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// FreeRTOS Task: Core 1 - Environmental Acquisition, Fusion & Telemetry (10 Hz / 1 Hz)
void vEnvFusionTask(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms = 10Hz
    uint8_t subSecondCounter = 0;

    for (;;) {
        // Update 6-Axis IMU orientation (10Hz)
        imuSensor.update();

        // Decay and update acoustic peak envelope (10Hz)
        AcousticData acousticData = piezoSensor.update();

        // Check RCWL hardware trigger timeout (hold trigger for 3 seconds after pulse)
        if (digitalRead(PIN_RCWL_INT) == HIGH) {
            rcwlTriggerActive = true;
            lastRcwlTriggerMs = millis();
        } else if (millis() - lastRcwlTriggerMs > 3000) {
            rcwlTriggerActive = false;
        }

        // 1Hz Slower Environmental Polling (CO2, Climate)
        subSecondCounter++;
        if (subSecondCounter >= 10) {
            subSecondCounter = 0;
            co2Sensor.read();
            climateSensor.read();
        }

        // Fetch Sensor States
        RadarData   radarData   = radarSensor.getData();
        IMUData     imuData     = imuSensor.getData();
        CO2Data     co2Data     = co2Sensor.getData();
        ClimateData climateData = climateSensor.getData();

        // Compute Multi-Modal Confidence Score
        FusionTelemetry fusionResult = fusionEngine.compute(
            radarSensor.getNormalizedPresenceScore(),
            rcwlTriggerActive,
            co2Sensor.getNormalizedCO2Score(),
            acousticData.normalizedAcousticScore,
            imuData.orientationWeight
        );

        // Control Locator Beacon (Buzzer & LED) on Confirmed Detection
        if (fusionResult.classification == STATE_CONFIRMED) {
            digitalWrite(PIN_BUZZER, (millis() / 500) % 2 == 0 ? HIGH : LOW);
            digitalWrite(PIN_LED_STROBE, HIGH);
        } else {
            digitalWrite(PIN_BUZZER, LOW);
            digitalWrite(PIN_LED_STROBE, LOW);
        }

        // Mock Battery Voltage (replace with ADC reading on battery divider)
        float batteryVolts = 3.92f;

        // Build and Broadcast JSON Telemetry
        String jsonTelemetry = commManager.buildJsonPayload(
            radarData,
            rcwlTriggerActive,
            imuData,
            co2Data,
            climateData,
            acousticData,
            fusionResult,
            batteryVolts
        );

        commManager.broadcast(jsonTelemetry);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup() {
    // Initialize Debug Serial
    Serial.begin(SERIAL_DEBUG_BAUD);
    delay(500);
    Serial.println(F("\n======================================================="));
    Serial.println(F("  DROPPABLE RECONNAISSANCE POD (DRP) — FIRMWARE INITIALIZING"));
    Serial.println(F("======================================================="));

    // Initialize Discrete GPIOs
    pinMode(PIN_RCWL_INT, INPUT);
    pinMode(PIN_RADAR_OUT, INPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED_STROBE, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED_STROBE, LOW);

    // Attach hardware interrupt for RCWL-0516
    attachInterrupt(digitalPinToInterrupt(PIN_RCWL_INT), isrRcwlMotion, RISING);

    // Initialize Radar (256000 baud UART2)
    radarSensor.begin(&SerialRadar, PIN_RADAR_RX, PIN_RADAR_TX, RADAR_UART_BAUD);
    Serial.println(F("[INIT] LD2410C 24GHz Radar Initialized on UART2"));

    // Initialize MPU6050 IMU
    if (imuSensor.begin(PIN_I2C_SDA, PIN_I2C_SCL)) {
        Serial.println(F("[INIT] MPU6050 6-Axis IMU Initialized via I2C"));
    } else {
        Serial.println(F("[WARN] MPU6050 Not Detected! Check I2C Wiring."));
    }

    // Initialize MH-Z19C NDIR CO2 (9600 baud UART1)
    co2Sensor.begin(&SerialMHZ, PIN_MHZ19_RX, PIN_MHZ19_TX);
    Serial.println(F("[INIT] MH-Z19C NDIR CO2 Sensor Initialized on UART1"));

    // Initialize DHT22 Climate Sensor & Piezo ADC
    climateSensor.begin();
    piezoSensor.begin();
    Serial.println(F("[INIT] DHT22 and Piezo Acoustic Pickup Initialized"));

    // Initialize Communications & AP
    commManager.begin();

    // Spawn Dual-Core FreeRTOS Tasks
    xTaskCreatePinnedToCore(
        vRadarDspTask,
        "RadarDspTask",
        4096,
        NULL,
        5, // High priority
        &hRadarDspTask,
        0  // Core 0
    );

    xTaskCreatePinnedToCore(
        vEnvFusionTask,
        "EnvFusionTask",
        8192,
        NULL,
        3, // Normal priority
        &hEnvFusionTask,
        1  // Core 1
    );

    Serial.println(F("[INIT] FreeRTOS Tasks Running on Core 0 & Core 1. System Ready."));
}

void loop() {
    // FreeRTOS handles task execution. Main loop remains idle.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
