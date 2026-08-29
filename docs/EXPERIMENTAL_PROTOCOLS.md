# Droppable Reconnaissance Pod (DRP) — Experimental Protocols & Benchmark

This document details the standardized testing protocols, logging formats, and validation criteria for evaluating the Droppable Reconnaissance Pod across diverse physical barriers and environmental conditions.

---

## 1. Safety, Ethics & Controlled Environment Guidelines

1. **Permitted Testing Facilities**: All tests must be executed strictly within authorized institutional spaces (college laboratory, controlled test rigs, or personal residential spaces with explicit written permission).
2. **Human Subject Protocol**: Human test targets must be voluntary team members who have consented to static sitting or walking intervals. No hazardous, prolonged, or distressing confinement is permitted.
3. **No Explosive or Hazard Testing**: Do not introduce flammable gases, toxic chemicals, or hazardous pyrotechnic materials. Environmental tests use safe carbon dioxide exhalation proxies (human breath in enclosed space).

---

## 2. Standardized Test Matrix & Evaluation Metrics

```
+----------------------------------------------------------------------------------------------------+
|                                    12-POINT EXPERIMENT VALIDATION MATRIX                           |
+----------------------------------------------------------------------------------------------------+
| ID     | Barrier Type / Condition | Target Protocol               | Distance Steps  | Pass Metric  |
+--------+--------------------------+-------------------------------+-----------------+--------------+
| EXP-01 | Free Air Baseline        | Walking human (1.0 m/s)       | 1, 2, 3, 4, 5 m | TPR >= 95%   |
| EXP-02 | Free Air Baseline        | Stationary seated human       | 1, 2, 3, 4 m    | TPR >= 90%   |
| EXP-03 | 12mm Plywood Partition   | Stationary seated human       | 0.5, 1, 2, 3 m  | TPR >= 80%   |
| EXP-04 | 15mm Gypsum Drywall      | Moving & stationary human     | 0.5, 1, 2, 3 m  | TPR >= 75%   |
| EXP-05 | 9-inch Indian Red Brick  | Moving human (walking)        | 0.5, 1.0, 1.5 m | Atten. curve |
| EXP-06 | 6-inch Fly Ash Brick     | Stationary human + exhalation | 0.5, 1.0, 1.5 m | Fusion >=70% |
| EXP-07 | RCC Concrete Plaster     | Moving human                  | 0.5, 1.0 m      | SNR Logged   |
| EXP-08 | Rubble / Metal Clutter   | Unoccupied shifting debris    | 1.0 m stand-off | FAR <= 15%   |
| EXP-09 | Tilt / Orientation Sweep | Pod @ 0°, 30°, 60°, 90° tilt  | 1.5 m (Plywood) | W_theta test |
| EXP-10 | Drop Shock Integrity     | 1.0 m drop onto sand/gravel   | Post-impact     | Health = PASS|
| EXP-11 | Multi-Target Separation  | 2 individuals spaced 1.5m     | 2.0 m stand-off | Bin isolation|
| EXP-12 | Battery vs Latency Curve | Duty cycle: 1s, 5s, 10s, 30s  | Continuous      | mAh vs Delay |
+--------+--------------------------+-------------------------------+-----------------+--------------+
```

---

## 3. Detailed Experimental Procedures

### EXP-01 & EXP-02: Free-Air Calibration Baseline
- **Objective**: Establish the unattenuated baseline receiver operating characteristic (ROC) of the 24 GHz LD2410C radar and RCWL-0516 Doppler sensor.
- **Procedure**:
  1. Position the DRP on a non-metallic tripod at 1.0 m height in an open room (minimum $6\text{m} \times 6\text{m}$).
  2. Mark radial distance intervals from 1.0 m to 5.0 m in 0.5 m increments.
  3. For EXP-01 (Motion): Test subject walks radially back and forth at $1.0\text{ m/s}$. Log 100 frames per distance step.
  4. For EXP-02 (Stationary): Test subject sits completely motionless on a wooden stool for 60 seconds at each distance step.
  5. Compute True Positive Rate (TPR) and signal-to-noise ratio per range gate.

### EXP-03 to EXP-07: Building Material Penetration Benchmark
- **Objective**: Quantify the 24 GHz attenuation coefficient and maximum effective penetration depth across Indian building materials.
- **Barrier Descriptions**:
  - *EXP-03*: 12 mm commercial grade marine plywood panel ($1.2\text{m} \times 2.4\text{m}$).
  - *EXP-04*: 15 mm gypsum dry-board standard partition.
  - *EXP-05*: Traditional 9-inch Indian red-clay burnt brick masonry with 12 mm cement mortar joint.
  - *EXP-06*: 6-inch compressed fly-ash brick wall section.
  - *EXP-07*: 100 mm reinforced cement concrete (RCC) slab test block.
- **Procedure**:
  1. Fix the pod flush against the barrier face ($d_{\text{pod-wall}} = 0\text{ cm}$) and offset ($d_{\text{pod-wall}} = 30\text{ cm}$).
  2. Position the human subject behind the barrier at distances of 0.5 m, 1.0 m, 1.5 m, 2.0 m, and 3.0 m.
  3. Record 200 telemetry packets at each distance step.
  4. Repeat without human presence to log baseline barrier backscatter clutter.

### EXP-09: Auto-Orientation & Tilt Compensation Test
- **Objective**: Validate the mathematical confidence penalty $W_\theta = \cos(\theta_{\text{tilt}})$ when the pod lands in an inclined orientation.
- **Procedure**:
  1. Secure the DRP on a variable-angle rotary stage behind a 12 mm plywood partition.
  2. Set target at fixed 1.5 m distance behind the partition.
  3. Rotate pod tilt angle $\theta$ across $0^\circ$ (normal bore-sight), $30^\circ$, $45^\circ$, $60^\circ$, and $90^\circ$ (perpendicular).
  4. Record the raw radar gate energy $S_{\text{radar}}$, the IMU computed tilt angle $\theta$, and the final fused confidence score $C(t)$.
  5. Confirm that the calculated confidence matches the theoretical curve $C(\theta) = C(0) \cdot \cos(\theta)$ within $\pm 8\%$.

---

## 4. Telemetry Logging Format

All experimental data is logged in a standardized comma-separated value (CSV) format:

```csv
timestamp_ms,exp_id,barrier_type,target_dist_m,target_state,tilt_deg,rcwl_trigger,radar_static_energy,radar_dynamic_energy,radar_peak_gate,co2_ppm,co2_rate_ppm_s,acoustic_peak_mv,temp_c,humidity_pct,fused_confidence_pct,ground_truth_present
104520,EXP-05,RED_BRICK_9IN,1.00,STATIONARY,14.2,0,48,12,2,485,0.85,32,28.4,62.1,64.8,1
```

---

## 5. Statistical Benchmark Formulas

- **True Positive Rate (Sensitivity)**:
  $$\text{TPR} = \frac{\text{TP}}{\text{TP} + \text{FN}}$$
- **False Alarm Rate (1 - Specificity)**:
  $$\text{FAR} = \frac{\text{FP}}{\text{FP} + \text{TN}}$$
- **Barrier Attenuation (dB)**:
  $$A_{\text{barrier}} = 10 \log_{10}\left( \frac{E_{\text{free-air}}(d)}{E_{\text{barrier}}(d)} \right)$$
- **F1-Score**:
  $$F_1 = 2 \cdot \frac{\text{Precision} \cdot \text{Recall}}{\text{Precision} + \text{Recall}} = \frac{2\text{TP}}{2\text{TP} + \text{FP} + \text{FN}}$$
