#ifndef DRP_CONFIG_H
#define DRP_CONFIG_H

#include <Arduino.h>

// ==========================================
// PIN DEFINITIONS (ESP32 DevKit V1 30-Pin)
// ==========================================
#define PIN_RADAR_RX          16  // ESP32 RX2 <- LD2410C TX
#define PIN_RADAR_TX          17  // ESP32 TX2 -> LD2410C RX
#define PIN_RADAR_OUT         26  // LD2410C Hardware OUT pin
#define PIN_RCWL_INT          27  // RCWL-0516 Doppler Wake Interrupt

#define PIN_I2C_SDA           21  // MPU6050 SDA
#define PIN_I2C_SCL           22  // MPU6050 SCL

#define PIN_MHZ19_RX          19  // ESP32 RX1 <- MH-Z19C TX
#define PIN_MHZ19_TX          18  // ESP32 TX1 -> MH-Z19C RX

#define PIN_DHT22_DATA        4   // DHT22 1-Wire Data
#define PIN_PIEZO_ADC         34  // LM358 / Piezo Analog Input (ADC1_CH6)

#define PIN_BUZZER            25  // Active 5V Buzzer Locator
#define PIN_LED_STROBE        33  // High-Brightness LED Strobe

// ==========================================
// SYSTEM PARAMETERS & THRESHOLDS
// ==========================================
#define POD_DEVICE_ID         "DRP-ALPHA-01"
#define FIRMWARE_VERSION       "v2.4.0-release"

// Serial Speeds
#define SERIAL_DEBUG_BAUD     115200
#define RADAR_UART_BAUD       256000
#define MHZ19_UART_BAUD       9600

// Sensor Fusion Feature Weights (Sum = 1.0)
#define WEIGHT_RADAR          0.45f
#define WEIGHT_DOPPLER        0.20f
#define WEIGHT_CO2            0.20f
#define WEIGHT_ACOUSTIC       0.15f

// Confidence Decision Bands (in %)
#define CONFIDENCE_BAND_UNCERTAIN   25.0f
#define CONFIDENCE_BAND_PROBABLE    50.0f
#define CONFIDENCE_BAND_CONFIRMED   75.0f

// CO2 Baseline & Rate Thresholds
#define CO2_BASELINE_PPM      420.0f
#define CO2_RATE_THRESHOLD    0.50f  // ppm/sec positive gradient to trigger score

// Acoustic / Piezo Thresholds (12-bit ADC 0-4095)
#define PIEZO_NOISE_FLOOR     150
#define PIEZO_PEAK_THRESHOLD  1200

// Wi-Fi Access Point Configuration
#define DRP_AP_SSID           "DRP_RECON_POD_01"
#define DRP_AP_PASS           "DisasterRescue2026"
#define DRP_WS_PORT           81

#endif // DRP_CONFIG_H
