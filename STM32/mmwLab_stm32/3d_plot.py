import csv
import math
import os

import matplotlib.pyplot as plt
import numpy as np


INPUT_CSV = "capture_log_normalized.csv"
OUTPUT_PNG = "polar_plots/antenna_lobes_3d.png"

AX1_STEPS_PER_REV = 72
AX2_STEPS_TOTAL = 18

USE_GLOBAL_NORMALIZATION = True
HPBW_THRESHOLD = 0.5


def theta_from_ax1_idx(ax1_idx):
    return math.radians((ax1_idx * 360.0) / AX1_STEPS_PER_REV)


def phi_from_ax2_idx(ax2_idx):
    return math.radians((ax2_idx * 180.0) / AX2_STEPS_TOTAL)


def read_rows(path):
    rows = []

    with open(path, "r", newline="") as f:
        reader = csv.DictReader(f)

        for row in reader:
            rows.append({
                "sweep_id": int(row["sweep_id"]),
                "ax1_idx": int(row["ax1_idx"]),
                "ax2_idx": int(row["ax2_idx"]),
                "ax1_pos": int(row["ax1_pos"]),
                "ax2_pos": int(row["ax2_pos"]),
                "i": int(row["i"]),
                "q": int(row["q"]),
                "power": float(row["power"]),
                "power_norm_sweep": float(row["power_norm_sweep"]),
            })

    return rows


def build_grid(rows):
    ax1_values = sorted(set(row["ax1_idx"] for row in rows))
    ax2_values = sorted(set(row["ax2_idx"] for row in rows))

    power_grid = np.full((len(ax2_values), len(ax1_values)), np.nan)

    ax1_lookup = {
        value: idx
        for idx, value in enumerate(ax1_values)
    }

    ax2_lookup = {
        value: idx
        for idx, value in enumerate(ax2_values)
    }

    if USE_GLOBAL_NORMALIZATION:
        max_power = max(row["power"] for row in rows)

        for row in rows:
            i = ax2_lookup[row["ax2_idx"]]
            j = ax1_lookup[row["ax1_idx"]]

            if max_power > 0:
                power_grid[i, j] = row["power"] / max_power
            else:
                power_grid[i, j] = 0.0
    else:
        for row in rows:
            i = ax2_lookup[row["ax2_idx"]]
            j = ax1_lookup[row["ax1_idx"]]
            power_grid[i, j] = row["power_norm_sweep"]

    return ax1_values, ax2_values, power_grid


def close_azimuth_grid(ax1_values, power_grid):
    first_col = power_grid[:, 0:1]
    power_grid_closed = np.hstack((power_grid, first_col))

    ax1_values_closed = ax1_values + [AX1_STEPS_PER_REV]

    return ax1_values_closed, power_grid_closed


def make_3d_surface(ax1_values, ax2_values, power_grid):
    theta = np.array([
        theta_from_ax1_idx(ax1_idx)
        for ax1_idx in ax1_values
    ])

    phi = np.array([
        phi_from_ax2_idx(ax2_idx)
        for ax2_idx in ax2_values
    ])

    theta_grid, phi_grid = np.meshgrid(theta, phi)

    r = np.nan_to_num(power_grid, nan=0.0)

    x = r * np.sin(phi_grid) * np.cos(theta_grid)
    y = r * np.sin(phi_grid) * np.sin(theta_grid)
    z = r * np.cos(phi_grid)

    return x, y, z, r


def compute_axial_hpbw(rows):
    ax2_power = {}

    for row in rows:
        ax2_idx = row["ax2_idx"]
        ax2_power[ax2_idx] = max(ax2_power.get(ax2_idx, 0.0), row["power"])

    if not ax2_power:
        return None

    max_power = max(ax2_power.values())

    if max_power <= 0:
        return None

    points = []

    for ax2_idx, power in sorted(ax2_power.items()):
        phi_deg = (ax2_idx * 180.0) / AX2_STEPS_TOTAL
        power_norm = power / max_power
        points.append((phi_deg, power_norm, ax2_idx))

    peak_index = max(range(len(points)), key=lambda i: points[i][1])

    left_phi = None
    right_phi = None

    for i in range(peak_index, 0, -1):
        p1 = points[i][1]
        p0 = points[i - 1][1]

        if p1 >= HPBW_THRESHOLD and p0 <= HPBW_THRESHOLD:
            phi1 = points[i][0]
            phi0 = points[i - 1][0]
            frac = (HPBW_THRESHOLD - p0) / (p1 - p0)
            left_phi = phi0 + frac * (phi1 - phi0)
            break

    for i in range(peak_index, len(points) - 1):
        p0 = points[i][1]
        p1 = points[i + 1][1]

        if p0 >= HPBW_THRESHOLD and p1 <= HPBW_THRESHOLD:
            phi0 = points[i][0]
            phi1 = points[i + 1][0]
            frac = (HPBW_THRESHOLD - p0) / (p1 - p0)
            right_phi = phi0 + frac * (phi1 - phi0)
            break

    if left_phi is not None and right_phi is not None:
        hpbw_deg = right_phi - left_phi
    else:
        hpbw_deg = None

    return {
        "hpbw_deg": hpbw_deg,
        "left_phi_deg": left_phi,
        "right_phi_deg": right_phi,
        "peak_phi_deg": points[peak_index][0],
        "peak_ax2_idx": points[peak_index][2],
        "points": points,
    }


def print_axial_hpbw(hpbw):
    if hpbw is None:
        print("axial HPBW could not be calculated")
        return

    print(f"axial peak ax2_idx: {hpbw['peak_ax2_idx']}")
    print(f"axial peak angle: {hpbw['peak_phi_deg']:.2f} deg")

    if hpbw["hpbw_deg"] is not None:
        print(f"axial HPBW: {hpbw['hpbw_deg']:.2f} deg")
        print(f"left half-power angle: {hpbw['left_phi_deg']:.2f} deg")
        print(f"right half-power angle: {hpbw['right_phi_deg']:.2f} deg")
    else:
        print("axial HPBW could not be fully measured")
        print("capture may not include both half-power crossings")


def plot_3d_lobes(x, y, z, r, hpbw=None):
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection="3d")

    ax.plot_surface(
        x,
        y,
        z,
        facecolors=plt.cm.viridis(r),
        linewidth=0,
        antialiased=True,
        shade=False,
    )

    mappable = plt.cm.ScalarMappable(cmap="viridis")
    mappable.set_array(r)
    mappable.set_clim(0, 1)

    fig.colorbar(
        mappable,
        ax=ax,
        shrink=0.65,
        label="normalized power",
    )

    title = "3D normalized antenna lobe model"

    if hpbw is not None and hpbw["hpbw_deg"] is not None:
        title += f"\naxial HPBW = {hpbw['hpbw_deg']:.2f} deg"

    ax.set_title(title)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")

    max_range = np.array([
        x.max() - x.min(),
        y.max() - y.min(),
        z.max() - z.min(),
    ]).max() / 2.0

    mid_x = (x.max() + x.min()) / 2.0
    mid_y = (y.max() + y.min()) / 2.0
    mid_z = (z.max() + z.min()) / 2.0

    ax.set_xlim(mid_x - max_range, mid_x + max_range)
    ax.set_ylim(mid_y - max_range, mid_y + max_range)
    ax.set_zlim(mid_z - max_range, mid_z + max_range)

    ax.view_init(elev=25, azim=45)

    plt.savefig(OUTPUT_PNG, dpi=300, bbox_inches="tight")
    print(f"3d plot written to {OUTPUT_PNG}")

    plt.show()


def main():
    if not os.path.exists(INPUT_CSV):
        print(f"missing input file: {INPUT_CSV}")
        return

    rows = read_rows(INPUT_CSV)

    if not rows:
        print("no rows found")
        return

    hpbw = compute_axial_hpbw(rows)

    ax1_values, ax2_values, power_grid = build_grid(rows)
    ax1_values, power_grid = close_azimuth_grid(ax1_values, power_grid)

    x, y, z, r = make_3d_surface(ax1_values, ax2_values, power_grid)

    print(f"loaded rows: {len(rows)}")
    print(f"axis 1 points: {len(ax1_values)}")
    print(f"axis 2 sweeps: {len(ax2_values)}")

    print_axial_hpbw(hpbw)

    plot_3d_lobes(x, y, z, r, hpbw)


if __name__ == "__main__":
    main()