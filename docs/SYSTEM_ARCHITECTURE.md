# Droppable Reconnaissance Pod (DRP) — System Architecture

This document specifies the technical architecture, mathematical formulations, firmware task partitioning, and multi-modal confidence engine for the Droppable Reconnaissance Pod (DRP).

---

## 1. High-Level Operational Architecture

```
+----------------------------------------------------------------------------------------------------+
|                                  DRP COMPLETE SYSTEM ARCHITECTURE                                  |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|  [ Aerial Delivery ]                                                                               |
|         |                                                                                          |
|         v (Impact Shock Absorbed by EVA Foam Enclosure)                                            |
|  +----------------------------------------------------------------------------------------------+  |
|  | DROPPABLE RECONNAISSANCE POD (Physical Device)                                               |  |
|  |                                                                                              |  |
|  |  +--------------------------------+      +------------------------------------------------+  |  |
|  |  | SENSING LAYER                  |      | POWER MANAGEMENT                               |  |  |
|  |  | - HLK-LD2410C (24GHz FMCW)     |      | - 18650 Li-ion (3.7V, 2600mAh)                 |  |  |
|  |  | - RCWL-0516 (3.18GHz Doppler)  |      | - TP4056 USB-C Charger & Protection            |  |  |
|  |  | - MPU6050 (6-Axis IMU)         |      | - AMS1117-3.3V & AMS1117-5.0V Boost-Buck Rails |  |  |
|  |  | - MH-Z19C (NDIR CO2)           |      +------------------------------------------------+  |  |
|  |  | - Piezo + LM358 (Acoustics)    |                                                          |  |
|  |  | - DHT22 (Temp & RH)            |                                                          |  |
|  |  +---------------+----------------+                                                          |  |
|  |                  | (I2C / UART / ADC / GPIO)                                                 |  |
|  |                  v                                                                           |  |
|  |  +----------------------------------------------------------------------------------------+  |  |
|  |  | COMPUTE & FUSION ENGINE (ESP32 Dual-Core @ 240MHz)                                     |  |  |
|  |  |                                                                                        |  |  |
|  |  |  [ Core 0: High-Rate Radar & DSP ]      [ Core 1: Environmental, Fusion & Comms ]      |  |  |
|  |  |  - FreeRTOS Task: radar_task (50Hz)     - FreeRTOS Task: env_task (1Hz)                |  |  |
|  |  |  - High-Speed UART Frame Parser         - MH-Z19 NDIR Exponential Moving Average      |  |  |
|  |  |  - Static / Dynamic Gate Filter        - Multi-Modal Confidence Scoring Engine        |  |  |
|  |  |  - Piezo ADC Peak Detection            - ESP-NOW Mesh & WebSocket Telemetry Server    |  |  |
|  |  +----------------------------------------------------------------------------------------+  |  |
|  +----------------------------------------------------------------------------------------------+  |
|         |                                                                                          |
|         | (ESP-NOW 2.4GHz Direct Link or Wi-Fi AP WebSocket)                                       |
|         v                                                                                          |
|  +----------------------------------------------------------------------------------------------+  |
|  | BASE STATION / OPERATOR DASHBOARD                                                            |  |
|  | - Real-Time Polar Radar Scope Visualizer                                                     |  |
|  | - 3D Orientation & Dynamic Detection Cone Display                                            |  |
|  | - Confidence Gauge (0-100%) with Uncertainty Bands                                           |  |
|  | - CSV Experiment Logger & Telemetry Stream                                                   |  |
|  +----------------------------------------------------------------------------------------------+  |
+----------------------------------------------------------------------------------------------------+
```

---

## 2. Multi-Modal Confidence Scoring Engine

A core contribution of the DRP architecture is the **anti-overclaiming confidence scoring engine**. Rather than outputting a binary `DETECTED / NOT DETECTED` flag (which produces unacceptable false alarm rates in rubble environments), the pod computes a bounded confidence score $C \in [0, 100]\%$.

### 2.1 Mathematical Formulation

The composite confidence score $C(t)$ at time sample $t$ is defined as:

$$C(t) = \min\left(100, \max\left(0, W_\theta \cdot \left[ w_r \cdot S_{\text{radar}}(t) + w_d \cdot S_{\text{doppler}}(t) + w_c \cdot S_{\text{CO2}}(t) + w_a \cdot S_{\text{acoustic}}(t) \right] + B_{\text{env}} \right)\right)$$

Where:
- $S_{\text{radar}}(t) \in [0, 100]$: Normalized radar presence energy derived from the HLK-LD2410C static and moving range gates:
  $$S_{\text{radar}}(t) = 0.6 \cdot E_{\text{static}}(r^*) + 0.4 \cdot E_{\text{moving}}(r^*)$$
  where $r^*$ is the range gate index with maximum signal-to-noise ratio.
- $S_{\text{doppler}}(t) \in \{0, 100\}$: Binary wake Doppler trigger from RCWL-0516 (100 if active, 0 if idle).
- $S_{\text{CO2}}(t) \in [0, 100]$: Gradient of carbon dioxide concentration over a sliding 30-second window:
  $$S_{\text{CO2}}(t) = \text{clamp}\left( \frac{\frac{d[\text{CO}_2]}{dt} - \delta_{\text{baseline}}}{\Delta_{\text{max}}} \cdot 100, 0, 100 \right)$$
  where $\delta_{\text{baseline}} \approx 0.5\text{ ppm/s}$ and $\Delta_{\text{max}} \approx 5.0\text{ ppm/s}$.
- $S_{\text{acoustic}}(t) \in [0, 100]$: Peak-normalized acoustic energy from structure-borne vibrations:
  $$S_{\text{acoustic}}(t) = \text{clamp}\left( \frac{V_{\text{peak}} - V_{\text{noise}}}{V_{\text{threshold}}} \cdot 100, 0, 100 \right)$$
- $W_\theta \in [0.1, 1.0]$: Orientation penalty factor derived from the IMU tilt angle relative to the horizon:
  $$W_\theta = \cos(\theta_{\text{tilt}})$$
  If the pod lands face down ($\theta_{\text{tilt}} = 90^\circ$), $W_\theta$ attenuates the radar confidence score to prevent false readings from ground clutter.
- $w_r = 0.45, w_d = 0.20, w_c = 0.20, w_a = 0.15$: Normalized feature weights ($\sum w_i = 1.0$).
- $B_{\text{env}}$: Baseline environmental temperature/humidity drift offset.

### 2.2 Decision Categorization Bands

| Confidence Range | Output Classification | Tactical Meaning & Recommended Operator Action |
|---|---|---|
| **0% – 24%** | `CLEAR / NO TARGET` | No biological signatures or structural anomalies detected. |
| **25% – 49%** | `UNCERTAIN / AMBIENT NOISE` | Weak reflection or isolated vibration spike; insufficient multi-modal consensus. |
| **50% – 74%** | `PROBABLE OCCUPANT` | Strong radar reflection or positive CO2 trend confirmed with Doppler motion. |
| **75% – 100%** | `CONFIRMED HUMAN PRESENCE` | Multi-modal agreement: radar static presence + CO2 accumulation + acoustic signature. |

---

## 3. FreeRTOS Dual-Core Firmware Task Allocation

To ensure microsecond-level timing accuracy for radar frame parsing without dropping Wi-Fi packets, the ESP32 firmware utilizes **FreeRTOS Dual-Core Task Pinning**:

```
+----------------------------------------------------------------------------------------------------+
|                                    ESP32 FREERTOS TASK SCHEDULING                                  |
+----------------------------------------------------------------------------------------------------+
| CORE 0 (Protocol & High-Speed DSP)               CORE 1 (Environmental, Fusion & UI)               |
|                                                                                                    |
| +-----------------------------------------+      +-----------------------------------------------+ |
| | Task: vRadarParserTask (Priority: 5)    |      | Task: vEnvironmentalTask (Priority: 2)        | |
| | - Period: 20 ms (50 Hz)                 |      | - Period: 1000 ms (1 Hz)                      | |
| | - Reads UART2 ring buffer (LD2410C)     |      | - Reads MH-Z19C CO2 UART1                     | |
| | - Validates Frame Header & Checksum     |      | - Reads DHT22 via 1-Wire                      | |
| | - Extracts Range Gate Energy Bins       |      | - Computes d(CO2)/dt Derivative               | |
| +-----------------------------------------+      +-----------------------------------------------+ |
|                      |                                                  |                          |
|                      v                                                  v                          |
| +-----------------------------------------+      +-----------------------------------------------+ |
| | Task: vAcousticSamplingTask (Prio: 4)   |      | Task: vConfidenceFusionTask (Priority: 3)     | |
| | - Period: 10 ms (100 Hz ADC)            |      | - Period: 100 ms (10 Hz)                      | |
| | - Peak envelope follower                |      | - Evaluates Multi-Modal Equation              | |
| | - Structure-borne knock counter         |      | - Generates Telemetry JSON Packet             | |
| +-----------------------------------------+      +-----------------------------------------------+ |
|                      |                                                  |                          |
|                      +-------------------------> Queue <----------------+                          |
|                                                  |                                                 |
|                                                  v                                                 |
|                                  +-----------------------------------------------+                 |
|                                  | Task: vTelemetryBroadcaster (Priority: 1)     |                 |
|                                  | - ESP-NOW Broadcast / WebSocket Server       |                 |
|                                  +-----------------------------------------------+                 |
+----------------------------------------------------------------------------------------------------+
```

---

## 4. State Machine & Power Duty-Cycling

To maximize operational runtime from a single 18650 Li-ion cell (2600 mAh), the firmware implements an adaptive finite state machine (FSM):

```
                       +-------------------+
                       |    INIT / POST    | (Self-test radar, IMU, CO2)
                       +---------+---------+
                                 |
                                 v
        +----------------------->+
        |                        |
        |              +---------v---------+
        |              |    SLEEP_SENTRY   | (ESP32 Light Sleep, RCWL active @ 3mA)
        |              +---------+---------+
        |                        |
        |                        | [RCWL Doppler Interrupt]
        |                        v
        |              +---------+---------+
        |              |    FAST_ACQUIRE   | (Power radar & IMU, 50Hz burst for 15s)
        |              +---------+---------+
        |                        |
        |                        | [Radar Target Detected]
        |                        v
        |              +---------+---------+
        |              |    DEEP_EVAL      | (Engage NDIR CO2 & Acoustic ADC for 60s)
        |              +---------+---------+
        |                        |
        |                        | [Confidence > 75%]
        |                        v
        |              +---------+---------+
        |              |   ALERT_BEACON    | (High-rate telemetry + buzzer beacon)
        |              +---------+---------+
        |                        |
        +---- [Timeout / No Motion] -----+
```

### Power Profile Across Operational States

| Operating State | Active Subsystems | Typical Current | Estimated Runtime (2600 mAh Cell) |
|---|---|---|---|
| **SLEEP_SENTRY** | ESP32 Light Sleep, RCWL-0516 | **4.2 mA** | **~619 hours (~25.8 days)** |
| **FAST_ACQUIRE** | ESP32 Active, LD2410C Radar, MPU6050 | **125 mA** | **~20.8 hours** |
| **DEEP_EVAL** | ESP32 Active, LD2410C, MH-Z19C, Piezo | **175 mA** | **~14.8 hours** |
| **ALERT_BEACON** | ESP32 Wi-Fi TX, Radar, Buzzer, LED | **240 mA** | **~10.8 hours** |
