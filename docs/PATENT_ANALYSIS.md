# Droppable Reconnaissance Pod (DRP) — Patent Prior Art & Novelty Analysis

This document provides a systematic patent landscape review, classification taxonomy, and freedom-to-operate (FTO) roadmap for discussing the intellectual property strategy with academic mentors and patent professionals.

---

## 1. Relevant Patent Classification Codes (CPC / IPC)

When conducting formal searches on **InPASS (Indian Patent Office)**, **Espacenet (EPO)**, or **WIPO Patentscope**, utilize the following International and Cooperative Patent Classifications:

| Classification Code | Description / Domain | Relevance to DRP Project |
|---|---|---|
| **G01S 13/887** | Radar systems specially adapted for specific applications: Through-the-wall imaging and detection. | Core radar sensing category. |
| **G01S 13/02** | Radar systems using reflection of electromagnetic waves (CW, FMCW, Impulse UWB). | Primary sensing physics. |
| **B64U 10/00** | Unmanned aerial vehicles (UAVs) / Drones. | Delivery and deployment platform. |
| **B64D 1/02** | Dropping or releasing payloads or sensor canisters from aerial vehicles. | Mechanical separation mechanism. |
| **G08B 21/10** | Alarm systems responsive to disaster situations (search and rescue, structural collapse). | Civilian application domain. |
| **G01V 1/00** | Seismology; Seismic or acoustic prospecting or detecting (structure-borne vibration). | Auxiliary acoustic sensing. |
| **G01N 33/004** | Investigating or analyzing gases: Determination of Carbon Dioxide ($CO_2$). | Auxiliary environmental sensing. |

---

## 2. In-Depth Prior Art Comparative Matrix

```
+---------------------------------------------------------------------------------------------------------------+
|                                      PATENT & PRIOR-ART LANDSCAPE SPECTRUM                                    |
+---------------------------------------------------------------------------------------------------------------+
| System / Patent ID | Key Claims & Methods                | Overlap with DRP         | DRP Novel Non-Infringing Delta  |
+--------------------+-------------------------------------+--------------------------+---------------------------------+
| **DRDO**           | Dual-band microwave radar +         | Through-wall human       | Divyachakshu is heavy (6-7 kg), |
| **Divyachakshu**   | thermal imaging for 20cm masonry    | detection in counter-    | tripod-mounted, ₹35L cost. DRP  |
| (India, 2017)      | penetration (tactical standoff).    | terrorism operations.    | is lightweight (<300g), drone-  |
|                    |                                     |                          | droppable, and sub-₹3,000.      |
+--------------------+-------------------------------------+--------------------------+---------------------------------+
| **US7528764B2**    | Multiple spaced-apart CW radar nodes| Multi-node through-wall  | Ground-fixed antennas; no aerial|
| (Zemany et al.)    | triangulating Doppler shift of      | motion detection.        | drop; no orientation tilt logic;|
|                    | moving targets behind barriers.     |                          | no CO2/acoustic fusion engine.  |
+--------------------+-------------------------------------+--------------------------+---------------------------------+
| **US9971019B2**    | Total-variation regularization      | Digital filtering of     | Algorithm-specific claim for TV |
| (Imaging Radar)    | denoising algorithm for synthetic   | radar backscatter.       | image reconstruction; DRP uses  |
|                    | through-wall radar arrays.          |                          | lightweight range-energy gates. |
+--------------------+-------------------------------------+--------------------------+---------------------------------+
| **US20190263489A1**| Parachute-stabilized droppable      | Droppable canister       | Survival kit payload / sonobuoy;|
| (Survitec Group)   | canister surviving water/land       | delivered from aircraft. | completely lacks active radar,  |
|                    | touchdown for survival gear delivery|                          | edge AI, or post-drop sensing.  |
+--------------------+-------------------------------------+--------------------------+---------------------------------+
| **DE3237485A1**    | Stepped-diameter deployable         | Droppable body housing   | Expired military submunition;   |
| (German Defense)   | microwave sensor assembly for       | microwave transceivers.  | lacks civilian rescue telemetry,|
|                    | submunition carriers.               |                          | self-orientation, and CO2 fusion|
+--------------------+-------------------------------------+--------------------------+---------------------------------+
```

---

## 3. High-Novelty Defensible Claims Vectors

To establish academic originality and potential utility patent claims, the DRP system targets narrow, method-level and system-level innovations rather than overbroad abstract claims:

### Claim Vector 1: Orientation-Aware Geometric Radar Confidence Weighting
*Method of dynamic radar confidence attenuation in an unanchored droppable pod, comprising:*
1. Acquiring 3-axis gravitational acceleration vectors via an onboard IMU following touchdown impact.
2. Determining the resting tilt angle $\theta_{\text{tilt}}$ between the radar aperture bore-sight and the horizontal plane.
3. Applying a continuous trigonometric attenuation function $W_\theta = \cos(\theta_{\text{tilt}})$ to the radar reflection energy score, suppressing spurious multi-path ground clutter without mechanical gimbal self-righting.

### Claim Vector 2: Multi-Modal Stillness Ambiguity Resolution via CO2 Derivative Fusion
*System and method for resolving static human occupancy behind high-loss barriers, comprising:*
1. Continuously monitoring 24 GHz static micro-motion energy bins within a defined range gate.
2. Simultaneously computing the time-derivative $\frac{d[\text{CO}_2]}{dt}$ from an NDIR optical gas chamber.
3. Triggering a positive occupant alert when the radar energy is in an ambiguous state ($40\% \le S_{\text{radar}} \le 60\%$) but $\frac{d[\text{CO}_2]}{dt} > \delta_{\text{threshold}}$, distinguishing living human respiration from inanimate moving reflectors (curtains, fans, water leaks).

### Claim Vector 3: Two-Tier Wake-on-Doppler Power Duty-Cycling Architecture
*Power-efficient autonomous reconnaissance method for constrained search and rescue pods, comprising:*
1. Maintaining the microcontroller and high-frequency radar in a low-power sleep state ($I < 5\text{ mA}$) with a wide-beam CW Doppler sensor configured as an asynchronous hardware interrupt.
2. Transitioning to a high-frequency (50 Hz) 24 GHz presence verification burst for a predetermined time window upon detecting motion.
3. Returning to sleep mode if static energy is below threshold, extending 18650 cell operating life beyond 72 hours.

---

## 4. Student Freedom-to-Operate (FTO) Search Checklist

Use this checklist during IP cell discussions:

- [x] **Google Patents Triage**: Verified through-wall, droppable canister, and radar fusion prior-art.
- [ ] **InPASS Indian Patent Search**:
  - Search Query 1: `Title: ("through wall" OR "through barrier") AND ("radar" OR "sensing")`
  - Search Query 2: `Abstract: ("drone" OR "UAV") AND ("droppable" OR "sensor pod")`
- [ ] **Espacenet Worldwide Search**:
  - Query: `CPC = G01S13/887 AND CPC = B64U`
- [ ] **WIPO Patentscope**:
  - Query: `FP:(("through wall radar") AND ("human detection") AND ("sensor fusion"))`
- [ ] **College IP Cell Consultation**: Review claim draft with university technology transfer office.
