# Droppable Reconnaissance Pod (DRP) — Through-Wall Human Sensing

<div align="center">

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Target Hardware](https://img.shields.io/badge/Hardware-ESP32%20%7C%2024GHz%20mmWave-orange.svg)
![Cost Ceiling](https://img.shields.io/badge/BOM%20Cost-%E2%82%B92%2C627%20INR%20(%3C%E2%82%B93k)-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platform-PlatformIO%20%7C%20Arduino%20%7C%20WebDashboard-blueviolet.svg)
![Application](https://img.shields.io/badge/Domain-Disaster%20Search%20%26%20Rescue-red.svg)

**A Low-Cost (~₹3,000 INR) Autonomous, Multi-Modal Sensor-Fused Pod for Post-Disaster Search & Rescue Reconnaissance**

*Designed for 2nd-Year Engineering Project Teams, Mentors, and University Evaluators.*

</div>

---

## 📑 Table of Contents
1. [Executive Summary & Problem Statement](#-executive-summary--problem-statement)
2. [How the System Works (Operational Workflow)](#-how-the-system-works-operational-workflow)
3. [System Architecture & Multi-Modal Fusion Math](#-system-architecture--multi-modal-fusion-math)
4. [Hardware Bill of Materials (BOM — Option B)](#-hardware-bill-of-materials-bom--option-b)
5. [Pinout & Electrical Wiring Matrix](#-pinout--electrical-wiring-matrix)
6. [Firmware Architecture & State Machine](#-firmware-architecture--state-machine)
7. [Live Mission Web Dashboard & Visualizer](#-live-mission-web-dashboard--visualizer)
8. [Python Simulation & Analysis Toolchain](#-python-simulation--analysis-toolchain)
9. [12-Point Experimental Benchmark & Material Testing](#-12-point-experimental-benchmark--material-testing)
10. [Prior Art & Patent Landscape Comparison](#-prior-art--patent-landscape-comparison)
11. [Quickstart & Flashing Guide](#-quickstart--flashing-guide)
12. [Project Repository Structure](#-project-repository-structure)
13. [Ethical Boundaries & Technical Disclaimers](#-ethical-boundaries--technical-disclaimers)

---

## 🎯 Executive Summary & Problem Statement

In collapsed structural disaster zones (earthquakes, structural failures, industrial explosions), search and rescue (SAR) teams face severe risks when entering unstable voids. Conventional through-wall imaging systems (e.g., **DRDO Divyachakshu**, Camero-Tech Xaver 400) cost **₹35 Lakhs to ₹2 Crores**, weigh 6–14 kg, and require manual placement on outer walls.

The **Droppable Reconnaissance Pod (DRP)** provides an autonomous, aerially-deployable, sub-₹3,000 INR search pod dropped from low-cost quadcopters into inaccessible structural voids.

### Key Capabilities & Highlights
- **Sub-₹3,000 Verified BOM**: Built with accessible COTS modules available across Indian electronics suppliers for **₹2,627 INR**.
- **24 GHz FMCW Presence Radar**: Extracts micro-motion and stillness energy bins via the HLK-LD2410C without requiring continuous gross movement.
- **Orientation-Aware Confidence Weighting**: An onboard MPU6050 calculates the post-drop resting tilt angle ($\theta_{\text{tilt}}$) and applies a mathematical penalty ($W_\theta = \cos\theta$) to suppress ground clutter without heavy mechanical gimbals.
- **Stillness Ambiguity Resolution**: Fuses NDIR CO2 concentration gradients ($d[\text{CO}_2]/dt$) and structure-borne acoustic vibrations to eliminate false alarms from inanimate motion.
- **Wake-on-Doppler Power Architecture**: Uses an ultra-low-power CW Doppler radar (RCWL-0516 @ 3 mA) as an interrupt trigger, extending battery life beyond **72 hours**.

---

## 🚀 How the System Works (Operational Workflow)

```
+----------------------------------------------------------------------------------------------------+
|                                    DRP OPERATIONAL MISSION LIFECYCLE                               |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|  [ 1. AERIAL PAYLOAD DROP ]                                                                        |
|  The delivery UAV navigates above a structural aperture (skylight, collapsed roof, window)         |
|  and releases the DRP pod via an electro-mechanical servo pin release.                             |
|                                                                                                    |
|  [ 2. IMPACT ABSORPTION & STABILIZATION ]                                                          |
|  The pod impacts the floor or rubble bed. High-density EVA shock foam dampens deceleration forces. |
|  The heavy 18650 Li-ion cell at the base provides passive self-righting center-of-gravity bias.    |
|                                                                                                    |
|  [ 3. POST-DROP IMU SELF-CALIBRATION ]                                                             |
|  The MPU6050 6-axis IMU measures the resting orientation $(\theta_{\text{tilt}})$.                 |
|  If tilted at an angle $\theta$, the firmware dynamically scales radar confidence by $W_\theta$.   |
|                                                                                                    |
|  [ 4. ASYNCHRONOUS WAKE-ON-DOPPLER SENTRY ]                                                        |
|  The ESP32 rests in low-power sentry mode. The 3 mA RCWL-0516 Doppler radar monitors a 360° sphere. |
|  Any motion in the room triggers a hardware interrupt, waking the high-rate 24 GHz presence radar. |
|                                                                                                    |
|  [ 5. MULTI-MODAL EVIDENCE FUSION ]                                                                |
|  - 24 GHz Radar measures static micro-motion energy bins.                                          |
|  - MH-Z19C NDIR sensor measures the accumulation rate of respired CO2 (+ppm/s).                   |
|  - Piezo transducer samples structure-borne acoustic taps and knocks.                             |
|  - The Multi-Modal Confidence Engine calculates a continuous score: $C \in [0, 100]\%$.             |
|                                                                                                    |
|  [ 6. TELEMETRY TRANSMISSION & RETRIEVAL BEACON ]                                                  |
|  The pod broadcasts real-time telemetry over ESP-NOW mesh or Wi-Fi AP to the operator dashboard.    |
|  If confidence exceeds 75% (Confirmed Occupant), a strobe LED and acoustic buzzer activate.        |
+----------------------------------------------------------------------------------------------------+
```

---

## 📐 System Architecture & Multi-Modal Fusion Math

The DRP implements an **anti-overclaiming confidence scoring engine** to replace misleading binary detections.

$$\Large C(t) = \min\left(100, \max\left(0, W_\theta \cdot \left[ w_r S_{\text{radar}} + w_d S_{\text{doppler}} + w_c S_{\text{CO2}} + w_a S_{\text{acoustic}} \right] \right)\right)$$

Where:
- $W_\theta = \cos(\theta_{\text{tilt}})$: Orientation penalty factor derived from IMU gravity vector.
- $S_{\text{radar}} = 0.65 E_{\text{static}} + 0.35 E_{\text{moving}}$: Radar presence energy (0–100%).
- $S_{\text{doppler}} \in \{0, 100\}$: Hardware Doppler wake flag.
- $S_{\text{CO2}} = \text{clamp}\left( \frac{[\text{CO}_2] - 420}{400} \cdot 50 + \frac{d[\text{CO}_2]/dt}{2.0} \cdot 50, 0, 100 \right)$: Exhalation gradient.
- $S_{\text{acoustic}} = \text{clamp}\left( \frac{V_{\text{peak}} - 50\text{mV}}{900\text{mV}} \cdot 100, 0, 100 \right)$: Structure-borne vibration score.
- Weights: $w_r = 0.45, w_d = 0.20, w_c = 0.20, w_a = 0.15$ ($\sum w_i = 1.0$).

### Classification Decision Bands
- `0% – 24%`: **CLEAR / NO TARGET** (No biological signatures)
- `25% – 49%`: **UNCERTAIN / AMBIENT NOISE** (Weak reflection or isolated vibration)
- `50% – 74%`: **PROBABLE OCCUPANT** (Strong radar reflection or positive CO2 trend)
- `75% – 100%`: **CONFIRMED HUMAN PRESENCE** (Multi-sensor agreement: radar + CO2 + acoustic)

---

## 🛒 Hardware Bill of Materials (BOM — Option B)

All components are verified Indian distributor stock (checked August 2026):

| Component | Part / Model | Role in DRP Pod | Verified Vendor | Price (INR) |
|---|---|---|---|---|
| **Microcontroller** | ESP32 DevKit V1 (30-pin) | Dual-Core 240MHz MCU, FreeRTOS, Wi-Fi | Robu.in / ElectroPi | **₹380** |
| **Presence Radar** | Hi-Link HLK-LD2410C-P | 24 GHz FMCW presence & static energy | Robu.in | **₹529** |
| **Wake Radar** | RCWL-0516 | 3.18 GHz CW Doppler wake interrupt (3mA) | ElectroPi.in | **₹28** |
| **6-Axis IMU** | GY-521 (MPU6050) | Tilt angle ($\theta_{\text{tilt}}$) & drop shock detection | Robu.in | **₹95** |
| **CO2 Gas Sensor** | Winsen MH-Z19C NDIR | Optical CO2 exhalation verification | Fab.to.Lab / Robu | **₹890** |
| **Climate Sensor** | DHT22 (AM2302) | Ambient temperature & humidity compensation | QuartzComponents | **₹165** |
| **Acoustic Transducer**| 27mm Piezo + LM358 | Structure-borne vibration & knock pickup | ElectronicsComp | **₹75** |
| **Battery Cell** | 18650 3.7V 2600mAh | High-capacity rechargeable power source | Robu.in | **₹140** |
| **Charger & Protection**| TP4056 USB-C + DW01A | CC/CV charge & over-discharge cut-off | ElectroPi.in | **₹35** |
| **Voltage Regulators** | AMS1117-5.0V / 3.3V | Dual-rail voltage regulation | Local Lab Stock | **₹30** |
| **Audio/Visual Beacon**| Active Buzzer + 10mm LED | Field retrieval beacon (85dB @ 10cm) | Robu.in | **₹30** |
| **Casing & Padding** | 3D Printed PETG + EVA Foam | High-impact protective pod shell | Campus Maker Lab | **₹150** |
| **Interconnects** | 30AWG silicone wire, headers | Circuit assembly & star ground | Local Lab Stock | **₹80** |
| **TOTAL BOM COST** | | | | **₹2,627** |

*Budget Margin Remaining: ₹373 INR within the ₹3,000 project ceiling.*

---

## 🔌 Pinout & Electrical Wiring Matrix

```
+-----------------------------------------------------------------------------------------+
|                                ESP32 PIN CONNECTIONS MATRIX                             |
+-----------------------------------------------------------------------------------------+
| ESP32 GPIO Pin   | Sensor / Subsystem Pin | Signal Type | Description                   |
+------------------+------------------------+-------------+-------------------------------+
| GPIO 16 (RX2)    | HLK-LD2410C TX         | UART RX     | 24 GHz Radar Data Stream      |
| GPIO 17 (TX2)    | HLK-LD2410C RX         | UART TX     | Radar Config & Thresholds     |
| GPIO 26          | HLK-LD2410C OUT        | Digital IN  | Discrete Presence Output      |
| GPIO 27          | RCWL-0516 OUT          | Digital INT | Wake-on-Doppler Interrupt     |
| GPIO 21 (SDA)    | MPU6050 SDA            | I2C Data    | 6-Axis Tilt & Impact Shock    |
| GPIO 22 (SCL)    | MPU6050 SCL            | I2C Clock   | 6-Axis Tilt & Impact Shock    |
| GPIO 19 (RX1)    | MH-Z19C TX             | UART RX     | CO2 Gas Concentration (ppm)   |
| GPIO 18 (TX1)    | MH-Z19C RX             | UART TX     | CO2 Command / Calibration     |
| GPIO 4           | DHT22 DATA             | 1-Wire      | Ambient Temp & Humidity       |
| GPIO 34 (ADC1)   | LM358 / Piezo OUT      | Analog ADC  | Structure-Borne Acoustic Wave |
| GPIO 25          | Active 5V Buzzer (+)   | Digital OUT | Rescue Locator Audio Strobe   |
| GPIO 33          | High-Bright LED (+)    | Digital OUT | Visual Optical Strobe         |
| 5.0V Boost Rail  | LD2410, MH-Z19, RCWL   | Power       | Boost Regulated 5V Rail       |
| 3.3V LDO Rail    | MPU6050, DHT22, ESP32  | Power       | Regulated 3.3V Rail           |
| GND              | Common Ground Bus      | Ground      | Star Ground Topology          |
+------------------+------------------------+-------------+-------------------------------+
```

---

## 💻 Firmware Architecture & State Machine

The firmware is structured in C++ using **PlatformIO / Arduino on FreeRTOS**:
- **Core 0 (`vRadarDspTask` @ 50 Hz)**: High-speed UART ring buffer parsing for the 24 GHz radar and high-rate ADC acoustic sampling.
- **Core 1 (`vEnvFusionTask` @ 10 Hz / 1 Hz)**: IMU orientation updates, NDIR CO2 derivative evaluation, multi-modal confidence engine, and telemetry broadcasting.

---

## 🖥️ Live Mission Web Dashboard & Visualizer

A responsive, high-fidelity dark-mode web dashboard is included in `dashboard/`:
- **Real-Time Polar Radar Scope**: Dynamic canvas visualizer showing distance arcs (1–5 m) and detected target blips.
- **3D IMU Artificial Horizon**: Visualizes pod tilt, pitch, and roll with dynamic $W_\theta$ compensation factor.
- **Confidence Gauge & Feature Breakdown**: Live breakdown of all 4 sensor modalities with uncertainty bands.
- **Environmental Status**: Real-time NDIR CO2 rate, temperature, humidity, and structure-borne vibration spikes.
- **Built-in Task 12 Scenario Simulator**: Instant 1-click simulation for Clear Room, Trapped Survivor, Rubble Noise, 60° Tilted Pod, and SOS Tapping.
- **CSV Data Exporter**: One-click download of timestamped telemetry logs.

```bash
# Launch the dashboard locally using any static web server:
cd dashboard
python -m http.server 8080
# Open http://localhost:8080 in your browser
```

---

## 🛠️ Python Simulation & Analysis Toolchain

The repository includes a complete Python toolchain in `tools/`:

### 1. Synthetic Telemetry Streamer (`tools/simulator.py`)
```bash
python tools/simulator.py --scenario trapped_survivor --rate 10.0
```

### 2. Experiment Data Logger (`tools/data_logger.py`)
```bash
# Record from physical ESP32 over serial:
python tools/data_logger.py --port COM3 --exp_id EXP-05 --barrier RED_BRICK_9IN --dist_m 1.0 --state STATIONARY

# Or pipe simulated stream directly:
python tools/simulator.py --scenario trapped_survivor | python tools/data_logger.py --stdin --exp_id EXP-03 --barrier PLYWOOD_12MM --dist_m 1.0
```

### 3. Statistical Benchmark Analysis (`tools/benchmark_analysis.py`)
```bash
python tools/benchmark_analysis.py --dir data_recordings --threshold 50.0
```

---

## 📊 12-Point Experimental Benchmark & Material Testing

| Test ID | Barrier / Condition | Target State | Distance Range | Pass Benchmark |
|---|---|---|---|---|
| **EXP-01** | Free Air (Baseline) | Walking human (1 m/s) | 1.0 m – 5.0 m | True Positive Rate $\ge 95\%$ |
| **EXP-02** | Free Air (Baseline) | Stationary seated human | 1.0 m – 4.0 m | Static Energy Stability $\ge 90\%$ |
| **EXP-03** | 12 mm Commercial Plywood | Stationary human | 0.5 m – 3.0 m | Detection Rate $\ge 80\%$ at 2.5 m |
| **EXP-04** | 15 mm Gypsum Drywall | Moving & stationary | 0.5 m – 3.0 m | Detection Rate $\ge 75\%$ at 2.0 m |
| **EXP-05** | 9-inch Indian Red Brick | Moving human | 0.5 m – 1.5 m | Attenuation Curve ($\sim 1.0\text{ m}$ limit) |
| **EXP-06** | 6-inch Fly Ash Brick | Stationary human + CO2 | 0.5 m – 1.5 m | Multi-modal Score $\ge 70\%$ |
| **EXP-07** | RCC Concrete Plaster | Moving human | 0.5 m – 1.0 m | Honest Backscatter Logging |
| **EXP-08** | Rubble / Metal Clutter | Shifting gravel / sheets | 1.0 m stand-off | False Alarm Rate $\le 15\%$ |
| **EXP-09** | Tilt Sweep ($0^\circ–90^\circ$) | Pod at $0^\circ, 30^\circ, 60^\circ, 90^\circ$ | 1.5 m stand-off | $W_\theta = \cos(\theta)$ curve validated |
| **EXP-10** | Drop Shock Impact | 1.0 m drop onto sand/gravel | Post-impact | Sensor Health = PASS |
| **EXP-11** | Multi-Target Separation | 2 targets spaced 1.5 m | 2.0 m stand-off | Gate Energy Isolation |
| **EXP-12** | Battery vs Latency | Duty cycle: 1s, 5s, 10s, 30s | Continuous | Energy Tradeoff Curve |

---

## 🔍 Prior Art & Patent Landscape Comparison

```
+----------------------------------------------------------------------------------------------------+
|                                    PRIOR ART COMPARATIVE SPECTRUM                                  |
+----------------------------------------------------------------------------------------------------+
| DRDO Divyachakshu (India)      Camero-Tech Xaver 400 (Israel)     Droppable Recon Pod (DRP)        |
| - 20-30 cm masonry penetration - 6-10 GHz IR-UWB array            - 24 GHz presence + NDIR CO2     |
| - ₹35 Lakhs unit cost          - ₹25 Lakhs - ₹1 Crore cost        - ₹2,627 INR (< ₹3k) budget      |
| - 6-7 kg tripod mounted        - 3.2 kg handheld tactical shield  - < 300g Drone-Droppable Pod     |
| - Manual exterior placement    - Manual exterior placement        - Autonomous interior deployment |
+----------------------------------------------------------------------------------------------------+
```

### Key Defensible Innovations
1. **Orientation-Aware Confidence Attenuation**: Suppressing ground clutter via $W_\theta = \cos(\theta_{\text{tilt}})$ without mechanical gimbals.
2. **CO2-Radar Stillness Ambiguity Resolution**: Distinguishing living human respiration from inanimate moving reflectors (curtains, fans, water leaks).
3. **India-Specific Material Benchmark**: Standardized 24 GHz attenuation dataset across Indian red clay brick, fly ash brick, and RCC plaster.

---

## ⚡ Quickstart & Flashing Guide

### Hardware Assembly
1. Connect sensor modules to the ESP32 according to the [Pinout Matrix](#-pinout--electrical-wiring-matrix).
2. Wire the 18650 battery through the TP4056 protection module and AMS1117 voltage regulators.
3. Mount the electronics inside the 3D-printed PETG enclosure with shock-dampening EVA foam.

### Flashing Firmware (PlatformIO)
```bash
# 1. Clone repository
git clone https://github.com/vanx007/Droppable-Reconnaissance-Pod-.git
cd Droppable-Reconnaissance-Pod-

# 2. Build and flash firmware to ESP32
cd firmware
pio run --target upload
pio device monitor --baud 115200
```

### Launching Mission Dashboard
```bash
# 3. Open dashboard in any web browser
cd ../dashboard
python -m http.server 8080
# Navigate to http://localhost:8080
```

---

## 📂 Project Repository Structure

```
Droppable-Reconnaissance-Pod/
├── README.md                          # Master documentation & user guide
├── .gitignore                         # Build and cache ignore configuration
│
├── docs/                              # Comprehensive Technical Documentation
│   ├── FEASIBILITY_REPORT.md          # Complete 13-part engineering & feasibility report
│   ├── SYSTEM_ARCHITECTURE.md         # Signal processing pipeline, FreeRTOS tasks & math
│   ├── HARDWARE_WIRING_BOM.md         # Component datasheets, wiring schematic & 3D casing
│   ├── EXPERIMENTAL_PROTOCOLS.md      # SOPs and metrics for 12 standardized test protocols
│   └── PATENT_ANALYSIS.md             # CPC classification, prior art matrix & FTO checklist
│
├── firmware/                          # ESP32 FreeRTOS Production Firmware
│   ├── platformio.ini                 # PlatformIO configuration file
│   ├── include/
│   │   ├── config.h                   # Pin definitions, thresholds, and weights
│   │   ├── sensors/
│   │   │   ├── ld2410_driver.h        # 24 GHz FMCW radar driver header
│   │   │   ├── mpu6050_driver.h       # 6-Axis IMU orientation header
│   │   │   ├── mhz19_driver.h         # NDIR CO2 gas sensor driver header
│   │   │   ├── dht22_driver.h         # Climate sensor driver header
│   │   │   └── piezo_driver.h         # Structure-borne acoustic driver header
│   │   ├── fusion/
│   │   │   └── confidence_engine.h    # Multi-modal fusion scoring engine header
│   │   └── telemetry/
│   │       └── comm_manager.h         # JSON telemetry & Wi-Fi broadcaster header
│   └── src/
│       ├── main.cpp                   # Dual-core task setup & FreeRTOS main loop
│       ├── sensors/
│       │   ├── ld2410_driver.cpp      # Radar frame parser & energy gate filter
│       │   ├── mpu6050_driver.cpp     # IMU tilt math & impact shock detection
│       │   └── mhz19_driver.cpp       # CO2 UART parser & rolling derivative
│       ├── fusion/
│       │   └── confidence_engine.cpp  # Confidence calculation implementation
│       └── telemetry/
│           └── comm_manager.cpp       # Telemetry serialization implementation
│
├── dashboard/                         # Live Mission Control Web Dashboard
│   ├── index.html                     # Responsive UI layout & visualizer
│   ├── style.css                      # Modern dark-mode glassmorphic styles
│   └── app.js                         # Polar radar scope & 3D IMU canvas renderer
│
├── tools/                             # Data Logging & Analysis Scripts
│   ├── simulator.py                   # Real-time synthetic telemetry packet generator
│   ├── data_logger.py                 # Structured CSV experiment logger
│   └── benchmark_analysis.py          # TPR, FAR, and F1-score metric analyzer
│
└── data_recordings/                   # Benchmark Experiment Logs
    └── sample_experiment_data.csv     # Sample Task 12 experiment log for instant testing
```

---

## ⚖️ Ethical Boundaries & Technical Disclaimers

1. **Civilian Search & Rescue Domain**: The DRP is designed exclusively for post-disaster search and rescue, structural collapse triage, and confined-space environmental monitoring.
2. **No Explosive Detection Claims**: Radar sensors detect electromagnetic reflectivity ($\Gamma$) and micro-motion; they cannot detect chemical composition or identify explosive substances.
3. **Intellectual Property Guidance**: Prior art analysis is an engineering landscape study. Formal patent filings require an interactive FTO search on InPASS and Espacenet conducted with university IP cells or registered patent attorneys.

---

<div align="center">
<b>Droppable Reconnaissance Pod (DRP) R&D Initiative</b><br>
<i>Engineering Innovation for Human Life Rescue in Confined Disaster Zones</i>
</div>
