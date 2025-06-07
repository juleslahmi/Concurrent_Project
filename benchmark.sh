#!/bin/bash

# Output CSV file
OUTPUT="results.csv"
echo "bodies,threads,time_sec" > "$OUTPUT"

# Define values to test
BODIES_LIST=(10 50 100 200 500 1000 2000)
THREADS_LIST=(1 2 4 6 8)

# Run tests
for bodies in "${BODIES_LIST[@]}"; do
  for threads in "${THREADS_LIST[@]}"; do
    echo "Running test: bodies=$bodies, threads=$threads"

    # Run the simulation and capture the output
    OUTPUT_LINE=$(./sim --testing --bodies "$bodies" --threads "$threads" 2>/dev/null | grep "Simulated")

    # Extract time using awk
    TIME_SEC=$(echo "$OUTPUT_LINE" | awk '{print $(NF-1)}')

    # Write to CSV
    echo "$bodies,$threads,$TIME_SEC" >> "$OUTPUT"
  done
done

echo "Benchmark complete. Results saved to $OUTPUT"
