#!/usr/bin/env python3
"""
Droppable Reconnaissance Pod (DRP) — Experiment Data Logger
Connects to ESP32 serial port or reads simulator stream, formats telemetry packets,
and logs structured CSV files for Task 12 experimental validation.
"""

import sys
import os
import time
import json
import csv
import argparse

try:
    import serial
except ImportError:
    serial = None

def main():
    parser = argparse.ArgumentParser(description="DRP Serial / JSON Data Logger")
    parser.add_argument("--port", type=str, default="COM3", help="Serial port (e.g. COM3 or /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--exp_id", type=str, default="EXP-01", help="Experiment ID (e.g. EXP-01 to EXP-12)")
    parser.add_argument("--barrier", type=str, default="FREE_AIR", help="Barrier type (FREE_AIR, PLYWOOD_12MM, DRYWALL_15MM, RED_BRICK_9IN, FLYASH_6IN)")
    parser.add_argument("--dist_m", type=float, default=1.0, help="Ground truth target distance in meters")
    parser.add_argument("--state", type=str, default="STATIONARY", help="Target state (STATIONARY, MOVING, ABSENT)")
    parser.add_argument("--output_dir", type=str, default="data_recordings", help="Output directory for CSV files")
    parser.add_argument("--stdin", action="store_true", help="Read JSON stream from STDIN instead of serial port")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)
    filename = f"{args.output_dir}/{args.exp_id}_{args.barrier}_{int(args.dist_m*100)}cm_{int(time.time())}.csv"

    fieldnames = [
        "timestamp_ms", "exp_id", "barrier_type", "target_dist_m", "target_state",
        "ground_truth_present", "vbat", "has_target", "radar_dist_cm", "static_energy",
        "moving_energy", "radar_score", "doppler_wake", "pitch_deg", "roll_deg",
        "tilt_deg", "w_theta", "co2_ppm", "co2_rate", "temp_c", "humidity_pct",
        "acoustic_peak_mv", "knocks", "fused_confidence_pct", "classification"
    ]

    print(f"[*] DRP Experiment Data Logger Active")
    print(f"[*] Experiment: {args.exp_id} | Barrier: {args.barrier} | Target Dist: {args.dist_m}m | Target State: {args.state}")
    print(f"[*] Logging to: {filename}\n")

    ground_truth_present = 0 if args.state.upper() == "ABSENT" else 1

    with open(filename, mode='w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        record_count = 0
        try:
            if args.stdin:
                stream = sys.stdin
            else:
                if serial is None:
                    print("[ERROR] pyserial is not installed. Use --stdin or run 'pip install pyserial'.")
                    sys.exit(1)
                stream = serial.Serial(args.port, args.baud, timeout=1.0)

            while True:
                line = stream.readline()
                if not line:
                    continue
                if isinstance(line, bytes):
                    line = line.decode('utf-8', errors='ignore').strip()
                else:
                    line = line.strip()

                if not line.startswith('{') or not line.endswith('}'):
                    continue

                try:
                    data = json.loads(line)
                except json.JSONDecodeError:
                    continue

                row = {
                    "timestamp_ms": data.get("uptime_ms", int(time.time() * 1000)),
                    "exp_id": args.exp_id,
                    "barrier_type": args.barrier,
                    "target_dist_m": args.dist_m,
                    "target_state": args.state,
                    "ground_truth_present": ground_truth_present,
                    "vbat": data.get("vbat", 0.0),
                    "has_target": 1 if data.get("radar", {}).get("has_target") else 0,
                    "radar_dist_cm": data.get("radar", {}).get("dist_cm", 0),
                    "static_energy": data.get("radar", {}).get("static_energy", 0),
                    "moving_energy": data.get("radar", {}).get("moving_energy", 0),
                    "radar_score": data.get("radar", {}).get("raw_score", 0.0),
                    "doppler_wake": 1 if data.get("doppler_wake") else 0,
                    "pitch_deg": data.get("imu", {}).get("pitch", 0.0),
                    "roll_deg": data.get("imu", {}).get("roll", 0.0),
                    "tilt_deg": data.get("imu", {}).get("tilt_deg", 0.0),
                    "w_theta": data.get("imu", {}).get("w_theta", 1.0),
                    "co2_ppm": data.get("environment", {}).get("co2_ppm", 420),
                    "co2_rate": data.get("environment", {}).get("co2_rate", 0.0),
                    "temp_c": data.get("environment", {}).get("temp_c", 25.0),
                    "humidity_pct": data.get("environment", {}).get("humidity_pct", 50.0),
                    "acoustic_peak_mv": data.get("acoustic", {}).get("peak_mv", 0),
                    "knocks": data.get("acoustic", {}).get("knocks", 0),
                    "fused_confidence_pct": data.get("fusion", {}).get("confidence_pct", 0.0),
                    "classification": data.get("fusion", {}).get("classification", "UNKNOWN")
                }

                writer.writerow(row)
                csvfile.flush()
                record_count += 1
                if record_count % 10 == 0:
                    print(f"[{record_count} pkts] Conf: {row['fused_confidence_pct']}% | Class: {row['classification']} | Dist: {row['radar_dist_cm']}cm | Tilt: {row['tilt_deg']}°")

        except KeyboardInterrupt:
            print(f"\n[*] Logging stopped. Total records saved: {record_count}")

if __name__ == "__main__":
    main()
