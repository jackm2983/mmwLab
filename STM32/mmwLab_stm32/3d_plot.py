import csv
import math
import os

import matplotlib.pyplot as plt
import numpy as np


INPUT_CSV = "capture_log_normalized.csv"
OUTPUT_PNG = "polar_plots/antenna_waterfall_3d.png"

AX1_STEPS_PER_REV = 72
AX2_STEP_DEG = 5.0
HPBW_THRESHOLD = 0.5
FLOOR_DB = -30  # clamp for display


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


def main():
    if not os.path.exists(INPUT_CSV):
        print(f"missing input file: {INPUT_CSV}")
        return
    rows = read_rows(INPUT_CSV)
    floor = min(r["power"] for r in rows)

    ax1_vals = sorted(set(r["ax1_idx"] for r in rows))
    ax2_vals = sorted(set(r["ax2_idx"] for r in rows))
    j_of = {v: j for j, v in enumerate(ax1_vals)}
    i_of = {v: i for i, v in enumerate(ax2_vals)}

    # power above floor, normalized to global peak
    grid = np.zeros((len(ax2_vals), len(ax1_vals)))
    for r in rows:
        grid[i_of[r["ax2_idx"]], j_of[r["ax1_idx"]]] = max(r["power"] - floor, 0.0)
    grid /= grid.max()

    grid_db = 10 * np.log10(np.clip(grid, 10 ** (FLOOR_DB / 10), None))

    # axes in degrees
    ax1_deg = np.array([v * 360.0 / AX1_STEPS_PER_REV for v in ax1_vals])
    ax2_deg = np.array([v * AX2_STEP_DEG for v in ax2_vals])
    X, Y = np.meshgrid(ax1_deg, ax2_deg)

    # axial hpbw on the co-pol cut (ax2_idx = 0)
    co = grid[i_of[0], :]
    peak = int(np.argmax(co))

    def cross(direction):
        idxs = range(peak, 0, -1) if direction == "left" else range(peak, len(co) - 1)
        for k in idxs:
            m = k - 1 if direction == "left" else k + 1
            if (co[k] - HPBW_THRESHOLD) * (co[m] - HPBW_THRESHOLD) <= 0 and co[k] != co[m]:
                frac = (HPBW_THRESHOLD - co[k]) / (co[m] - co[k])
                return ax1_deg[k] + frac * (ax1_deg[m] - ax1_deg[k])
        return None

    left, right = cross("left"), cross("right")
    hpbw = right - left if (left and right) else None

    os.makedirs(os.path.dirname(OUTPUT_PNG), exist_ok=True)
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection="3d")

    surf = ax.plot_surface(X, Y, grid_db, cmap="viridis", linewidth=0,
                           antialiased=True, rcount=80, ccount=120, vmin=FLOOR_DB, vmax=0)

    fig.colorbar(surf, ax=ax, shrink=0.6, label="normalized power (dB)")

    # trace the co-pol ridge in red so the main lobe stands out
    ax.plot(ax1_deg, np.zeros_like(ax1_deg), grid_db[i_of[0], :],
            color="red", lw=2.5, label="co-pol cut (ax2=0)")

    ax.set_xlabel("ax1 angle (deg)")
    ax.set_ylabel("polarization ax2 (deg)")
    ax.set_zlabel("normalized power (dB)")
    ax.set_zlim(FLOOR_DB, 0)
    ax.set_ylim(90, 0)  # co-pol toward viewer

    title = "polarization waterfall: lobe decay vs cross-pol angle"
    if hpbw is not None:
        title += f"\naxial HPBW = {hpbw:.1f} deg (peak {ax1_deg[peak]:.0f} deg)"
    ax.set_title(title)
    ax.view_init(elev=28, azim=-60)
    ax.legend(loc="upper right")

    plt.savefig(OUTPUT_PNG, dpi=300, bbox_inches="tight")
    print(f"plot written to {OUTPUT_PNG}")
    if hpbw is not None:
        print(f"axial HPBW: {hpbw:.2f} deg, peak {ax1_deg[peak]:.1f} deg, edges {left:.1f}/{right:.1f}")
    plt.show()


if __name__ == "__main__":
    main()