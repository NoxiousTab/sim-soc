import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def plot_results(csv_file):
    df = pd.read_csv(csv_file)

    plt.figure(figsize=(8, 5))
    plt.plot(df["id"], df["system_cum_energy_j"], marker="o", label="System Cumulative Energy (J)")
    plt.xlabel("Task ID")
    plt.ylabel("Energy (J)")
    plt.title("System Energy Consumption Over Tasks")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    plt.figure(figsize=(8, 5))
    for core, group in df.groupby("core"):
        plt.plot(group["id"], group["core_cum_energy_j"], marker="o", label=f"Core {core}")
    plt.xlabel("Task ID")
    plt.ylabel("Energy (J)")
    plt.title("Per-Core Energy Consumption")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    plt.figure(figsize=(10, 5))

    for core, group in df.groupby("core"):
        times = np.cumsum(group["time_s"])  # cumulative time per core
        energies = group["core_cum_energy_j"]
        plt.step(times, energies, where='post', label=f"Core {core}")

    plt.xlabel("Time (s)")
    plt.ylabel("Cumulative Energy (J)")
    plt.title("Time-Series Per-Core Energy Consumption")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_results.py <results_csv>")
    else:
        plot_results(sys.argv[1])
