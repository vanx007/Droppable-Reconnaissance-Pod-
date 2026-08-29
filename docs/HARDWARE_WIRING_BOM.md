# Droppable Reconnaissance Pod (DRP) — Hardware, Wiring & Bill of Materials

This guide provides complete electrical schematics, pin mappings, power tree calculations, and mechanical assembly instructions for the Droppable Reconnaissance Pod (Option B - Balanced ₹3,000 Target).

---

## 1. Verified Bill of Materials (BOM)

All components are standard commercial-off-the-shelf (COTS) modules available across Indian electronics distributors (checked August 2026).

| # | Item Description | Part Number / Model | Key Specifications | Quantity | Target Vendor | Unit Cost (INR) | Total Cost (INR) |
|---|---|---|---|---|---|---|---|
| 1 | Microcontroller DevKit | ESP32 DevKit V1 (30-pin) | Dual-core Xtensa 240MHz, 520KB SRAM, Wi-Fi / BLE | 1 | Robu.in / ElectroPi | ₹380 | ₹380 |
| 2 | 24 GHz FMCW Radar | Hi-Link HLK-LD2410C-P | 24.00–24.25 GHz, Static & Moving energy gates | 1 | Robu.in | ₹529 | ₹529 |
| 3 | CW Microwave Doppler | RCWL-0516 | 3.18 GHz CW, 360° motion detection, 3mA draw | 1 | ElectroPi.in | ₹28 | ₹28 |
| 4 | 6-Axis Inertial Sensor | GY-521 (MPU6050) | 3-axis Accelerometer ($\pm 16g$) + 3-axis Gyroscope | 1 | Robu.in | ₹95 | ₹95 |
| 5 | NDIR Carbon Dioxide Sensor | Winsen MH-Z19C | 0–5000 ppm range, NDIR optical chamber, UART | 1 | Fab.to.Lab / Robu | ₹890 | ₹890 |
| 6 | Climate Sensor | DHT22 (AM2302) | -40 to 80°C ($\pm 0.5^\circ\text{C}$), 0–100% RH ($\pm 2\%$) | 1 | QuartzComponents | ₹165 | ₹165 |
| 7 | Acoustic Transducer | 27mm Piezo Element + LM358 | Structure-borne vibration pickup, high-impedance | 1 | ElectronicsComp | ₹75 | ₹75 |
| 8 | Primary Li-ion Battery | 18650 3.7V 2600mAh | High-discharge rechargeable cylindrical cell | 1 | Robu.in | ₹140 | ₹140 |
| 9 | Charge & Protection Board | TP4056 with DW01A | Type-C 1A CC/CV charger + Over-discharge cut-off | 1 | ElectroPi.in | ₹35 | ₹35 |
| 10 | Power Rail Regulators | AMS1117-5.0V & AMS1117-3.3V | Low-dropout linear voltage regulators | 2 | Local / Online | ₹30 | ₹30 |
| 11 | Location Beacon & Buzzer | 5V Active Buzzer + 10mm LED | 85 dB @ 10cm audio beacon + ultra-bright white LED | 1 | Robu.in | ₹30 | ₹30 |
| 12 | Mechanical Casing | 3D-Printed PLA + High-Density Foam | Spherical / Dodecahedron capsule with EVA bumpers | 1 | Campus Maker Lab | ₹150 | ₹150 |
| 13 | Interconnects & Fasteners | 30AWG silicone wire, headers | Jumper wires, toggle switch, M3 nylon standoffs | 1 Set | Local Lab Stock | ₹70 | ₹70 |
| | **TOTAL PROJECT HARDWARE COST** | | | | | | **₹2,627** |

*Budget Ceiling: ₹3,000 INR. Remaining Margin: ₹373 INR.*

---

## 2. Electrical Pinout & Interconnect Table

```
+---------------------------------------------------------------------------------------+
|                                ESP32 PIN CONNECTIONS MATRIX                           |
+---------------------------------------------------------------------------------------+
| ESP32 GPIO Pin | Sensor / Module Pin    | Signal Type | Description / Subsystem       |
+----------------+------------------------+-------------+-------------------------------+
| GPIO 16 (RX2)  | HLK-LD2410C TX         | UART RX     | 24 GHz Radar Target Telemetry |
| GPIO 17 (TX2)  | HLK-LD2410C RX         | UART TX     | Radar Config & Sensitivity    |
| GPIO 26        | HLK-LD2410C OUT        | Digital IN  | Hardware Presence Trigger     |
| GPIO 27        | RCWL-0516 OUT          | Digital INT | Wake-on-Doppler Interrupt     |
| GPIO 21 (SDA)  | MPU6050 SDA            | I2C Data    | Orientation / Drop Shock      |
| GPIO 22 (SCL)  | MPU6050 SCL            | I2C Clock   | Orientation / Drop Shock      |
| GPIO 19 (RX1)  | MH-Z19C TX             | UART RX     | CO2 Concentration (ppm)       |
| GPIO 18 (TX1)  | MH-Z19C RX             | UART TX     | CO2 Command / Calibration     |
| GPIO 4         | DHT22 DATA             | 1-Wire      | Ambient Temperature & Humidity|
| GPIO 34 (ADC1) | LM358 / Piezo OUT      | Analog ADC  | Structure-Borne Acoustic Wave |
| GPIO 25        | Active Buzzer (+)      | Digital OUT | Rescue Retrieval Audio Beacon |
| GPIO 33        | High-Bright LED (+)    | Digital OUT | Visual Optical Strobe Beacon  |
| 5V Rail        | LD2410, MH-Z19, RCWL   | Power       | Regulated 5.0V Boost Line     |
| 3.3V Rail      | MPU6050, DHT22, ESP32  | Power       | Regulated 3.3V Line           |
| GND            | Common Ground Bus      | Ground      | Star Ground Configuration     |
+----------------+------------------------+-------------+-------------------------------+
```

---

## 3. Power Architecture & Voltage Regulation Schematic

```
+----------------------------------------------------------------------------------------------------+
|                                    DRP POWER DISTRIBUTION TREE                                     |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
| [ 18650 Li-ion Battery (3.7V - 4.2V, 2600mAh) ]                                                    |
|                   |                                                                                |
|                   v                                                                                |
| [ TP4056 Protection Module (Overcharge, Over-discharge cutoff @ 2.9V) ]                            |
|                   |                                                                                |
|                   +-------------------------------------------------+                              |
|                   |                                                 |                              |
|                   v (VBAT: 3.7V - 4.2V)                             v (VBAT)                       |
|   [ MT3608 Step-Up Boost (Fixed 5.0V) ]           [ AMS1117-3.3V LDO Regulator ]                   |
|                   |                                                 |                              |
|                   v                                                 v                              |
|        +---------------------+                           +---------------------+                   |
|        | 5.0V POWER BUS      |                           | 3.3V POWER BUS      |                   |
|        +---------------------+                           +---------------------+                   |
|        | - HLK-LD2410C Radar |                           | - ESP32 VDD Pin     |                   |
|        | - MH-Z19C NDIR CO2  |                           | - MPU6050 IMU       |                   |
|        | - RCWL-0516 Doppler |                           | - DHT22 Temp/RH     |                   |
|        | - LM358 Piezo Amp   |                           | - Status Indicator  |                   |
|        | - Active 5V Buzzer  |                           +---------------------+                   |
|        +---------------------+                                                                     |
+----------------------------------------------------------------------------------------------------+
```

---

## 4. Mechanical Enclosure & Drop-Impact Protection

```
+----------------------------------------------------------------------------------------------------+
|                                 DRP MECHANICAL ENCLOSURE DESIGN                                    |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|                             Outer Impact Shell (3D Printed PETG / PLA)                             |
|                           +--------------------------------------------+                           |
|                          /                                              \                          |
|                         /    +------------------------------------+      \                         |
|                        |    /   High-Density EVA Shock Foam Ring   \      |                        |
|                        |   |                                        |     |                        |
|                        |   |   +--------------------------------+   |     |                        |
|                        |   |   |   Internal Electronic Core:    |   |     |                        |
|                        |   |   |   - ESP32 + MPU6050 (Center)   |   |     |                        |
|                        |   |   |   - 18650 Cell (Bottom Weight) |   |     |                        |
|                        |   |   |   - MH-Z19C (Vented Top)       |   |     |                        |
|                        |   |   +--------------------------------+   |     |                        |
|                        |   |                                        |     |                        |
|                        |   |   +--------------------------------+   |     |                        |
|                        |    \  | 24GHz Radar Aperture (Thin 1mm)|  /      |                        |
|                         \    +-\--------------------------------/-+      /                         |
|                          \      \ Piezo Base Coupler to Ground /        /                          |
|                           +------\----------------------------/--------+                           |
|                                   \                          /                                     |
|                                    +------------------------+                                      |
+----------------------------------------------------------------------------------------------------+
```

### Key Mechanical Specifications:
1. **Self-Righting Geometric Bias**: The heavy 18650 battery (~48 grams) is mounted at the extreme bottom of the pod capsule, creating a low center of gravity (CoG) that encourages passive upright orientation upon impact.
2. **RF Transmissive Radar Window**: The radar aperture utilizes a 1.0 mm thin PLA/PETG wall directly in front of the LD2410C patch antennas to minimize 24 GHz signal attenuation ($<0.5\text{ dB}$).
3. **Structure-Borne Acoustic Coupling**: The 27 mm piezo transducer is mounted to a bottom contact stud that physically protrudes 2 mm beyond the outer shell, guaranteeing direct mechanical contact with the rubble/floor surface upon landing.
4. **Vented Atmospheric Port**: The top dome features filtered labyrinth vents allowing ambient air circulation to the MH-Z19C NDIR CO2 sensor and DHT22 while preventing dust/debris ingress.
