import csv
import math
import queue
import sys
import threading
import time
from datetime import datetime

import matplotlib.pyplot as plt
import serial


PORT = "COM3"
BAUD = 9600
CSV_PATH = "capture_log.csv"


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

    ax1_idx = int(parts[1])
    ax2_idx = int(parts[2])
    ax1_pos = int(parts[3])
    ax2_pos = int(parts[4])
    i_val = int(parts[5])
    q_val = int(parts[6])

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
    max_power = 1.0

    csv_file = open(CSV_PATH, "w", newline="")
    writer_csv = csv.DictWriter(
        csv_file,
        fieldnames=[
            "timestamp",
            "ax1_idx",
            "ax2_idx",
            "ax1_pos",
            "ax2_pos",
            "i",
            "q",
            "power",
            "power_norm",
        ],
    )
    writer_csv.writeheader()


    plt.ion()
    fig = plt.figure()
    ax = fig.add_subplot(111, projection="polar")
    line_plot, = ax.plot([], [], marker="o")

    ax.set_title("normalized antenna transmit power")
    ax.set_ylim(0, 1.05)
    ax.grid(True)

    current_ax2 = None

    try:
        while not stop_event.is_set():
            updated = False

            while not line_queue.empty():
                line = line_queue.get()
                row = parse_cap_line(line)

                if row is None:
                    continue

                max_power = max(max_power, row["power"])
                row["power_norm"] = row["power"] / max_power

                rows.append(row)
                writer_csv.writerow(row)
                csv_file.flush()

                current_ax2 = row["ax2_idx"]
                updated = True

            if updated:
                plot_rows = [r for r in rows if r["ax2_idx"] == current_ax2]

                theta = [
                    math.radians((r["ax1_idx"] * 360.0) / 72.0)
                    for r in plot_rows
                ]

                radius = [r["power_norm"] for r in plot_rows]

                line_plot.set_data(theta, radius)
                ax.set_title(f"normalized antenna transmit power, axis 2 index {current_ax2}")

                fig.canvas.draw()
                fig.canvas.flush_events()

            time.sleep(0.02)

    except KeyboardInterrupt:
        pass

    finally:
        stop_event.set()
        ser.close()
        csv_file.close()
        plt.ioff()
        plt.show()


if __name__ == "__main__":
    main()