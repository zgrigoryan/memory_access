import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file into a DataFrame
df = pd.read_csv('results.csv')

# Plot the aligned and unaligned execution times
plt.figure(figsize=(10, 6))

plt.plot(df['Run'], df['AlignedTime'], label='Aligned Time', color='blue')
plt.plot(df['Run'], df['UnalignedTime'], label='Unaligned Time', color='red')

plt.xlabel('Run')
plt.ylabel('Time (seconds)')
plt.title('Execution Time Comparison: Aligned vs Unaligned Access')
plt.legend()

plt.grid(True)

# Save the plot as a PNG image
plt.savefig('execution_time_comparison.png')

# Show the plot
plt.show()
