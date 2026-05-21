import csv
import math
import queue
import threading
import time
from datetime import datetime

import matplotlib.pyplot as plt
import serial


PORT = "COM3"
BAUD = 9600

RAW_CSV_PATH = "capture_log_raw.csv"
NORMALIZED_CSV_PATH = "capture_log_normalized.csv"

AX1_STEPS_PER_REV = 72


def serial_reader(ser, line_queue, stop_event):
    while not stop_event.is_set():
        try:
            raw = ser.readline()
            if not raw:
                continue

            line = raw.decode("utf-8", errors="replace").strip()

            if line:
                print(line)
                line_queue.put(line)

        except serial.SerialException as e:
            print(f"serial error: {e}")
            stop_event.set()
            break


def serial_writer(ser, stop_event):
    import msvcrt

    while not stop_event.is_set():
        if msvcrt.kbhit():
            ch = msvcrt.getch()

            if ch in (b"\x00", b"\xe0"):
                msvcrt.getch()
                continue

            if ch == b"\x03":
                stop_event.set()
                break

            ser.write(ch)

        time.sleep(0.005)


def parse_cap_line(line):
    parts = line.split(",")

    if len(parts) != 7 or parts[0] != "cap":
        return None

    try:
        ax1_idx = int(parts[1])
        ax2_idx = int(parts[2])
        ax1_pos = int(parts[3])
        ax2_pos = int(parts[4])
        i_val = int(parts[5])
        q_val = int(parts[6])
    except ValueError:
        return None

    power = math.sqrt(i_val * i_val + q_val * q_val)

    return {
        "timestamp": datetime.now().isoformat(timespec="milliseconds"),
        "ax1_idx": ax1_idx,
        "ax2_idx": ax2_idx,
        "ax1_pos": ax1_pos,
        "ax2_pos": ax2_pos,
        "i": i_val,
        "q": q_val,
        "power": power,
    }


def theta_from_ax1_idx(ax1_idx):
    return math.radians((ax1_idx * 360.0) / AX1_STEPS_PER_REV)


def write_normalized_csv(rows, path):
    if not rows:
        return

    sweep_max = {}

    for row in rows:
        sweep_id = row["sweep_id"]
        sweep_max[sweep_id] = max(sweep_max.get(sweep_id, 0.0), row["power"])

    normalized_rows = []

    for row in rows:
        out = row.copy()
        max_power = sweep_max[row["sweep_id"]]

        if max_power > 0:
            out["power_norm_sweep"] = out["power"] / max_power
            out["power_db_norm"] = 20.0 * math.log10(out["power_norm_sweep"])
        else:
            out["power_norm_sweep"] = 0.0
            out["power_db_norm"] = None

        normalized_rows.append(out)

    fieldnames = [
        "timestamp",
        "sweep_id",
        "ax1_idx",
        "ax2_idx",
        "ax1_pos",
        "ax2_pos",
        "i",
        "q",
        "power",
        "power_norm_sweep",
        "power_db_norm",
    ]

    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(normalized_rows)


def main():
    line_queue = queue.Queue()
    stop_event = threading.Event()

    ser = serial.Serial(PORT, BAUD, timeout=0.1)

    reader = threading.Thread(
        target=serial_reader,
        args=(ser, line_queue, stop_event),
        daemon=True,
    )

    writer = threading.Thread(
        target=serial_writer,
        args=(ser, stop_event),
        daemon=True,
    )

    reader.start()
    writer.start()

    rows = []

    sweep_id = -1
    last_ax2_idx = None
    current_sweep_id = None
    current_ax2_idx = None

    raw_fieldnames = [
        "timestamp",
        "sweep_id",
        "ax1_idx",
        "ax2_idx",
        "ax1_pos",
        "ax2_pos",
        "i",
        "q",
        "power",
    ]

    raw_csv_file = open(RAW_CSV_PATH, "w", newline="")
    raw_writer = csv.DictWriter(raw_csv_file, fieldnames=raw_fieldnames)
    raw_writer.writeheader()

    plt.ion()

    fig = plt.figure()
    ax = fig.add_subplot(111, projection="polar")

    line_plot, = ax.plot([], [], marker="o")

    ax.set_title("normalized antenna transmit power")
    ax.set_ylim(0, 1.05)
    ax.grid(True)

    try:
        while not stop_event.is_set():
            updated = False

            while not line_queue.empty():
                line = line_queue.get()
                row = parse_cap_line(line)

                if row is None:
                    continue

                if row["ax2_idx"] != last_ax2_idx:
                    sweep_id += 1
                    last_ax2_idx = row["ax2_idx"]
                    print(f"new sweep {sweep_id}, axis 2 index {row['ax2_idx']}")

                row["sweep_id"] = sweep_id

                rows.append(row)
                raw_writer.writerow(row)
                raw_csv_file.flush()

                current_sweep_id = sweep_id
                current_ax2_idx = row["ax2_idx"]
                updated = True

            if updated and current_sweep_id is not None:
                plot_rows = [
                    r for r in rows
                    if r["sweep_id"] == current_sweep_id
                ]

                if plot_rows:
                    sweep_max_power = max(r["power"] for r in plot_rows)

                    theta = [
                        theta_from_ax1_idx(r["ax1_idx"])
                        for r in plot_rows
                    ]

                    if sweep_max_power > 0:
                        radius = [
                            r["power"] / sweep_max_power
                            for r in plot_rows
                        ]
                    else:
                        radius = [0.0 for _ in plot_rows]

                    line_plot.set_data(theta, radius)

                    ax.set_title(
                        f"normalized antenna transmit power, "
                        f"sweep {current_sweep_id}, axis 2 index {current_ax2_idx}"
                    )

                    fig.canvas.draw()
                    fig.canvas.flush_events()

            time.sleep(0.02)

    except KeyboardInterrupt:
        pass

    finally:
        stop_event.set()

        try:
            ser.close()
        except serial.SerialException:
            pass

        raw_csv_file.close()

        write_normalized_csv(rows, NORMALIZED_CSV_PATH)

        print(f"raw csv written to {RAW_CSV_PATH}")
        print(f"normalized csv written to {NORMALIZED_CSV_PATH}")

        plt.ioff()
        plt.show()


if __name__ == "__main__":
    main()