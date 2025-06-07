import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv("results.csv")

# Group by body count for separate lines
grouped = df.groupby("bodies")

# Set up the plot
plt.figure(figsize=(10, 6))
plt.yscale("log")

# Plot each group
for bodies, group in grouped:
    plt.plot(group["threads"], group["time_sec"], marker="o", label=f"{bodies} bodies")

# Add labels, title, and legend
plt.title("Simulation Time vs Threads per Body Count")
plt.xlabel("Number of Threads")
plt.ylabel("Time (seconds)")
plt.legend(title="Bodies")
plt.grid(True)

# Save to PNG (in the current directory)
plt.tight_layout()
plt.savefig("results_plot.png")

# Optional: remove this in WSL
# plt.show()
