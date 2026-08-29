#!/usr/bin/env python3
"""
Droppable Reconnaissance Pod (DRP) — Benchmark Analysis & Metric Tool
Reads recorded experiment CSV logs, calculates TPR, FAR, Precision, Recall, F1-Score,
and produces statistical benchmark summaries across Indian construction barriers.
"""

import sys
import os
import glob
import argparse
import math

def analyze_csv(filepath, threshold_pct=50.0):
    total_records = 0
    tp, fp, tn, fn = 0, 0, 0, 0
    conf_sum = 0.0
    radar_energy_sum = 0.0
    co2_rate_sum = 0.0

    with open(filepath, 'r') as f:
        lines = f.readlines()
        if len(lines) < 2:
            return None

        header = [h.strip() for h in lines[0].split(',')]
        gt_idx = header.index("ground_truth_present") if "ground_truth_present" in header else -1
        conf_idx = header.index("fused_confidence_pct") if "fused_confidence_pct" in header else -1
        static_idx = header.index("static_energy") if "static_energy" in header else -1
        co2_rate_idx = header.index("co2_rate") if "co2_rate" in header else -1

        for line in lines[1:]:
            parts = [p.strip() for p in line.split(',')]
            if len(parts) != len(header):
                continue

            total_records += 1
            gt = int(float(parts[gt_idx])) if gt_idx != -1 else 1
            conf = float(parts[conf_idx]) if conf_idx != -1 else 0.0
            conf_sum += conf

            if static_idx != -1:
                radar_energy_sum += float(parts[static_idx])
            if co2_rate_idx != -1:
                co2_rate_sum += float(parts[co2_rate_idx])

            pred = 1 if conf >= threshold_pct else 0

            if gt == 1 and pred == 1:
                tp += 1
            elif gt == 0 and pred == 1:
                fp += 1
            elif gt == 0 and pred == 0:
                tn += 1
            elif gt == 1 and pred == 0:
                fn += 1

    tpr = (tp / (tp + fn)) * 100.0 if (tp + fn) > 0 else 0.0
    far = (fp / (fp + tn)) * 100.0 if (fp + tn) > 0 else 0.0
    precision = (tp / (tp + fp)) * 100.0 if (tp + fp) > 0 else 0.0
    recall = tpr
    f1 = (2 * precision * recall) / (precision + recall) if (precision + recall) > 0 else 0.0
    avg_conf = conf_sum / total_records if total_records > 0 else 0.0
    avg_static = radar_energy_sum / total_records if total_records > 0 else 0.0

    return {
        "file": os.path.basename(filepath),
        "total": total_records,
        "tp": tp, "fp": fp, "tn": tn, "fn": fn,
        "tpr": tpr, "far": far,
        "precision": precision,
        "f1": f1,
        "avg_confidence": avg_conf,
        "avg_static_energy": avg_static
    }

def main():
    parser = argparse.ArgumentParser(description="DRP Experiment Metric Analysis")
    parser.add_argument("--dir", type=str, default="data_recordings", help="Directory containing CSV recordings")
    parser.add_argument("--threshold", type=float, default=50.0, help="Confidence threshold percentage for positive detection")
    args = parser.parse_args()

    files = glob.glob(os.path.join(args.dir, "*.csv"))
    if not files:
        print(f"[*] No CSV recordings found in '{args.dir}'.")
        print("[*] Run 'python tools/data_logger.py' or generate test data to analyze.")
        return

    print("=" * 95)
    print(f" DROPPABLE RECONNAISSANCE POD — STATISTICAL BENCHMARK ANALYSIS (Threshold: {args.threshold}%)")
    print("=" * 95)
    print(f"{'Experiment Log':<35} | {'Pkts':<5} | {'TPR %':<7} | {'FAR %':<7} | {'Prec %':<7} | {'F1':<6} | {'Avg Conf':<8}")
    print("-" * 95)

    for fpath in files:
        res = analyze_csv(fpath, args.threshold)
        if res:
            print(f"{res['file']:<35} | {res['total']:<5} | {res['tpr']:<7.1f} | {res['far']:<7.1f} | {res['precision']:<7.1f} | {res['f1']:<6.2f} | {res['avg_confidence']:<8.1f}%")

    print("=" * 95)

if __name__ == "__main__":
    main()
