import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_data(file_paths, labels):
    # Set up the plot with 3x2 grid of subplots
    fig, axes = plt.subplots(4, 2, figsize=(12, 18))  # 3 rows, 2 columns
    axes = axes.flatten()  # Flatten the 2D array of axes to make indexing easier

    # Iterate through each CSV file and its corresponding label
    for i, (file_path, label) in enumerate(zip(file_paths, labels)):
        df = pd.read_csv(file_path)

        # Convert Timestamp_Sent to datetime
        df['Timestamp_Sent'] = pd.to_datetime(df['Timestamp_Sent'])

        # Calculate time difference in milliseconds between consecutive rows
        df['Time_Difference_ms'] = df['Timestamp_Sent'].diff().dt.total_seconds() * 1000

        # Filter out negative time differences
        df = df[df['Time_Difference_ms'] >= 0]

        # Create a hexbin plot for each file
        ax = axes[i]
        hb = ax.hexbin(df['No_Detected'], df['No_Classified'], gridsize=30, cmap='Blues', mincnt=1)

        # Add a title and labels for each subplot
        ax.set_title(f"Heatmap: {label}")
        ax.set_xlabel("No_Detected")
        ax.set_ylabel("No_Classified")
        
        # Add a color bar to show the density of points
        fig.colorbar(hb, ax=ax, label='Number of Points')

    # Adjust layout to avoid overlap
    plt.tight_layout()
    plt.show()

# Example usage
file_paths = ["performance_timeL640.csv", "performance_timeL1080.csv",
              "performance_timeM640.csv", "performance_timeM1080.csv",
              "performance_timeS640.csv", "performance_timeS1080.csv",
              "performance_timeN640.csv", "performance_timeN1080.csv"]
labels = ["L640", "L1080", "M640", "M1080", "S640", "S1080", "N640", "N1080"]

# Call the function
plot_data(file_paths, labels)
