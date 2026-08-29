#!/usr/bin/env python3
"""
Droppable Reconnaissance Pod (DRP) — Synthetic Telemetry Simulator
Generates and broadcasts simulated DRP JSON telemetry packets for testing dashboards
and data pipelines without physical hardware.
"""

import time
import json
import math
import random
import argparse

SCENARIOS = {
    "clear": {
        "description": "Unoccupied open room (No target)",
        "radar": {"has_target": False, "dist_cm": 0, "static_energy": 2, "moving_energy": 1},
        "doppler": False,
        "co2_base": 420, "co2_rate": 0.0,
        "acoustic_base": 25,
        "tilt_deg": 0.0
    },
    "trapped_survivor": {
        "description": "Stationary breathing human behind 15mm drywall/plywood",
        "radar": {"has_target": True, "dist_cm": 140, "static_energy": 78, "moving_energy": 12},
        "doppler": False,
        "co2_base": 580, "co2_rate": 1.15,
        "acoustic_base": 60,
        "tilt_deg": 12.0
    },
    "walking": {
        "description": "Moving subject walking in room",
        "radar": {"has_target": True, "dist_cm": 220, "static_energy": 25, "moving_energy": 88},
        "doppler": True,
        "co2_base": 440, "co2_rate": 0.20,
        "acoustic_base": 45,
        "tilt_deg": 5.0
    },
    "rubble_clutter": {
        "description": "Shifting rubble / metal sheet clutter (False alarm test)",
        "radar": {"has_target": True, "dist_cm": 80, "static_energy": 35, "moving_energy": 30},
        "doppler": False,
        "co2_base": 421, "co2_rate": 0.01,
        "acoustic_base": 220,
        "tilt_deg": 20.0
    },
    "tilted_landing": {
        "description": "Pod landed at 55 degree severe tilt after drop",
        "radar": {"has_target": True, "dist_cm": 150, "static_energy": 65, "moving_energy": 10},
        "doppler": False,
        "co2_base": 460, "co2_rate": 0.40,
        "acoustic_base": 30,
        "tilt_deg": 55.0
    }
}

def generate_packet(scenario_name, t_sec):
    scen = SCENARIOS.get(scenario_name, SCENARIOS["clear"])
    
    # Radar calculations
    r_cfg = scen["radar"]
    has_target = r_cfg["has_target"]
    dist = r_cfg["dist_cm"] + int(math.sin(t_sec * 0.5) * 10) if has_target else 0
    static_e = min(100, max(0, r_cfg["static_energy"] + random.randint(-3, 3))) if has_target else random.randint(0, 4)
    moving_e = min(100, max(0, r_cfg["moving_energy"] + random.randint(-4, 4))) if has_target else random.randint(0, 3)
    
    radar_score = min(100.0, (static_e * 0.65) + (moving_e * 0.35)) if has_target else 0.0
    doppler_score = 100.0 if scen["doppler"] else 0.0
    
    # CO2 calculations
    co2_ppm = scen["co2_base"] + int(math.sin(t_sec * 0.1) * 20)
    co2_rate = scen["co2_rate"] + random.uniform(-0.05, 0.05)
    co2_score = min(100.0, max(0.0, ((co2_ppm - 420) / 400.0) * 50.0 + (co2_rate / 2.0) * 50.0))
    
    # Acoustic calculations
    acoustic_mv = scen["acoustic_base"] + random.randint(0, 15)
    acoustic_score = min(100.0, max(0.0, ((acoustic_mv - 50) / 900.0) * 100.0))
    
    # IMU orientation calculations
    tilt = scen["tilt_deg"] + random.uniform(-0.5, 0.5)
    w_theta = max(0.15, math.cos(math.radians(tilt)))
    
    # Weighted multi-modal fusion
    composite = (0.45 * radar_score) + (0.20 * doppler_score) + (0.20 * co2_score) + (0.15 * acoustic_score)
    final_conf = min(100.0, max(0.0, composite * w_theta))
    
    # Classification
    if final_conf >= 75.0:
        cls_text = "CONFIRMED HUMAN PRESENCE"
        state_code = 3
    elif final_conf >= 50.0:
        cls_text = "PROBABLE OCCUPANT"
        state_code = 2
    elif final_conf >= 25.0:
        cls_text = "UNCERTAIN / AMBIENT NOISE"
        state_code = 1
    else:
        cls_text = "CLEAR / NO TARGET"
        state_code = 0

    packet = {
        "device_id": "DRP-ALPHA-SIM",
        "fw": "v2.4.0-sim",
        "uptime_ms": int(t_sec * 1000),
        "vbat": round(3.95 - (t_sec * 0.0001), 2),
        "radar": {
            "has_target": has_target,
            "is_static": static_e > 30,
            "is_moving": moving_e > 30,
            "dist_cm": dist,
            "static_energy": static_e,
            "moving_energy": moving_e,
            "raw_score": round(radar_score, 1)
        },
        "doppler_wake": scen["doppler"],
        "imu": {
            "pitch": round(tilt * 0.8, 1),
            "roll": round(tilt * 0.6, 1),
            "tilt_deg": round(tilt, 1),
            "w_theta": round(w_theta, 3),
            "impact_shock": False
        },
        "environment": {
            "co2_ppm": co2_ppm,
            "co2_rate": round(co2_rate, 2),
            "temp_c": round(26.2 + math.sin(t_sec * 0.02) * 0.5, 1),
            "humidity_pct": round(58.0 + math.cos(t_sec * 0.02) * 1.5, 1)
        },
        "acoustic": {
            "peak_mv": acoustic_mv,
            "knocks": 1 if acoustic_mv > 500 else 0,
            "score": round(acoustic_score, 1)
        },
        "fusion": {
            "confidence_pct": round(final_conf, 1),
            "state_code": state_code,
            "classification": cls_text
        }
    }
    return packet

def main():
    parser = argparse.ArgumentParser(description="DRP Synthetic Telemetry Streamer")
    parser.add_argument("--scenario", choices=list(SCENARIOS.keys()), default="trapped_survivor", help="Simulation scenario")
    parser.add_argument("--rate", type=float, default=10.0, help="Publish rate in Hz")
    parser.add_argument("--duration", type=float, default=0, help="Duration in seconds (0 = infinite)")
    args = parser.parse_args()

    print(f"[*] Starting DRP Telemetry Simulator...")
    print(f"[*] Scenario: [{args.scenario.upper()}] — {SCENARIOS[args.scenario]['description']}")
    print(f"[*] Rate: {args.rate} Hz | Press Ctrl+C to stop.\n")

    start_time = time.time()
    dt = 1.0 / args.rate

    try:
        while True:
            t_sec = time.time() - start_time
            if args.duration > 0 and t_sec > args.duration:
                break
            packet = generate_packet(args.scenario, t_sec)
            print(json.dumps(packet))
            time.sleep(dt)
    except KeyboardInterrupt:
        print("\n[*] Simulation terminated by user.")

if __name__ == "__main__":
    main()
