import csv
import math
import os

import matplotlib.pyplot as plt
import numpy as np


INPUT_CSV = "capture_log_normalized.csv"
OUTPUT_PNG = "antenna_polar_2d.png"

AX1_STEPS_PER_REV = 72
HPBW_THRESHOLD = 0.5  # -3 db in linear power

# which ax2 sweeps to draw. 0 = co-pol, 18 = full cross-pol (90 deg)
COPOL_AX2 = 0
CROSSPOL_AX2 = 18


def read_rows(path):
    rows = []
    with open(path, "r", newline="") as f:
        for r in csv.DictReader(f):
            rows.append({
                "ax1_idx": int(r["ax1_idx"]),
                "ax2_idx": int(r["ax2_idx"]),
                "power": float(r["power"]),
            })
    return rows


def estimate_floor(rows):
    return min(r["power"] for r in rows)


def get_cut(rows, ax2_idx, floor, norm):
    cut = sorted((r for r in rows if r["ax2_idx"] == ax2_idx), key=lambda r: r["ax1_idx"])
    ang = np.array([math.radians(r["ax1_idx"] * 360.0 / AX1_STEPS_PER_REV) for r in cut])
    p = np.array([max(r["power"] - floor, 0.0) for r in cut]) / norm
    # close the loop
    ang = np.append(ang, ang[0])
    p = np.append(p, p[0])
    return ang, p


def to_db(p):
    return 10 * np.log10(np.clip(p, 1e-4, None))


def compute_hpbw(rows, ax2_idx, floor):
    cut = sorted((r for r in rows if r["ax2_idx"] == ax2_idx), key=lambda r: r["ax1_idx"])
    ang = np.array([r["ax1_idx"] * 360.0 / AX1_STEPS_PER_REV for r in cut])
    p = np.array([max(r["power"] - floor, 0.0) for r in cut])
    pmax = p.max()
    if pmax <= 0:
        return None
    pn = p / pmax
    peak = int(np.argmax(pn))

    def cross(direction):
        idxs = range(peak, 0, -1) if direction == "left" else range(peak, len(pn) - 1)
        for i in idxs:
            j = i - 1 if direction == "left" else i + 1
            if (pn[i] - HPBW_THRESHOLD) * (pn[j] - HPBW_THRESHOLD) <= 0 and pn[i] != pn[j]:
                frac = (HPBW_THRESHOLD - pn[i]) / (pn[j] - pn[i])
                return ang[i] + frac * (ang[j] - ang[i])
        return None

    left, right = cross("left"), cross("right")
    hpbw = (right - left) if (left is not None and right is not None) else None
    return {"hpbw": hpbw, "left": left, "right": right, "peak": ang[peak]}


def main():
    if not os.path.exists(INPUT_CSV):
        print(f"missing input file: {INPUT_CSV}")
        return
    rows = read_rows(INPUT_CSV)
    floor = estimate_floor(rows)

    # normalize both cuts to the co-pol peak so cross-pol rejection is visible
    copol_peak = max(r["power"] - floor for r in rows if r["ax2_idx"] == COPOL_AX2)

    co_ang, co_p = get_cut(rows, COPOL_AX2, floor, copol_peak)
    cx_ang, cx_p = get_cut(rows, CROSSPOL_AX2, floor, copol_peak)

    h = compute_hpbw(rows, COPOL_AX2, floor)

    output_dir = os.path.dirname(OUTPUT_PNG)

    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
    fig, ax = plt.subplots(figsize=(9, 9), subplot_kw={"projection": "polar"})

    # plot in db
    co_db = to_db(co_p)
    cx_db = to_db(cx_p)
    floor_db = -30  # plot floor

    ax.plot(co_ang, np.clip(co_db, floor_db, 0), color="#1f77b4", lw=2, label="co-pol (ax2=0)")
    ax.plot(cx_ang, np.clip(cx_db, floor_db, 0), color="#d62728", lw=2, label="cross-pol (ax2=90)")
    ax.fill(co_ang, np.clip(co_db, floor_db, 0), color="#1f77b4", alpha=0.1)

    # -3 db ring
    ax.plot(np.linspace(0, 2 * np.pi, 200), [-3] * 200, color="gray", ls="--", lw=1, label="-3 dB")

    # mark hpbw edges
    if h and h["hpbw"] is not None:
        for edge in (h["left"], h["right"]):
            ax.plot([math.radians(edge)] * 2, [floor_db, -3], color="green", lw=1.2)
        ax.plot([math.radians(h["peak"])] * 2, [floor_db, 0], color="black", lw=1, ls=":")

    ax.set_theta_zero_location("N")
    ax.set_theta_direction(-1)
    ax.set_rlim(floor_db, 0)
    ax.set_rticks([-30, -20, -10, -3, 0])
    ax.set_rlabel_position(135)
    ax.set_title("antenna radiation pattern (normalized, dB)", pad=20)
    ax.legend(loc="lower left", bbox_to_anchor=(-0.1, -0.1))

    if h and h["hpbw"] is not None:
        cross_rej = co_db.max() - cx_db.max()
        txt = (f"axial HPBW = {h['hpbw']:.1f} deg\n"
               f"peak = {h['peak']:.1f} deg\n"
               f"cross-pol rejection = {cross_rej:.1f} dB")
        ax.text(np.radians(45), floor_db - 6, txt, fontsize=10,
                bbox=dict(boxstyle="round", fc="white", ec="gray"))

    plt.savefig(OUTPUT_PNG, dpi=300, bbox_inches="tight")
    print(f"plot written to {OUTPUT_PNG}")
    if h and h["hpbw"] is not None:
        print(f"axial HPBW: {h['hpbw']:.2f} deg")
        print(f"peak: {h['peak']:.2f} deg")
        print(f"-3dB edges: {h['left']:.2f} / {h['right']:.2f} deg")
    plt.show()


if __name__ == "__main__":
    main()