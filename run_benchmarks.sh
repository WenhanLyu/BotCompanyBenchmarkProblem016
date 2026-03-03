#!/bin/bash

# Performance Benchmark Runner for B+ Tree File Storage
# Measures baseline performance before cache implementation
# and verifies improvements after cache is added

set -e

EXECUTABLE="${1:-./code}"
OUTPUT_FILE="${2:-performance_results.md}"

echo "=== B+ Tree Performance Benchmark Suite ==="
echo "Executable: $EXECUTABLE"
echo "Output: $OUTPUT_FILE"
echo ""

# Ensure executable exists and is built
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable $EXECUTABLE not found"
    echo "Building..."
    make clean && make
fi

# Compile stress test if needed
if [ ! -f ./stress_test ]; then
    echo "Compiling stress test program..."
    g++ -std=c++17 -O2 -Wall -Wextra stress_test.cpp -o stress_test
fi

# Create results header
cat > "$OUTPUT_FILE" << 'EOF'
# B+ Tree Performance Benchmark Results

**Date**: $(date +"%Y-%m-%d %H:%M:%S")
**Platform**: $(uname -s) $(uname -m)
**Compiler**: $(g++ --version | head -1)

## Test Methodology

This benchmark suite measures the performance of the B+ Tree file storage implementation
using a realistic mix of operations:
- 60% INSERT operations
- 30% FIND operations  
- 10% DELETE operations

Each test uses a fixed random seed (42) for reproducibility. Keys are varied in length
(8-64 bytes) to simulate realistic workloads. The same key can have multiple values,
and operations may reference non-existent entries.

### Performance Targets

For successful completion, the implementation must meet:
- **Time**: ≤4 seconds for 300K operations (with 20% safety margin under 5s limit)
- **Memory**: ≤60 MiB (with safety margin under 64 MiB limit)
- **Disk**: No specific limit, but should be reasonable (<100 MiB)

---

## Benchmark Results

EOF

# Function to run a single benchmark
run_benchmark() {
    local num_ops=$1
    local label=$2
    
    echo "Running benchmark: $label ($num_ops operations)..."
    
    # Clean up data file
    rm -f data_file
    
    # Generate test data
    ./stress_test $num_ops 42 "$EXECUTABLE" > /tmp/bench_output_$num_ops.txt 2>&1
    
    # Extract results
    local time_sec=$(grep "Execution Time:" /tmp/bench_output_$num_ops.txt | awk '{print $3}')
    local avg_us=$(grep "Avg Time per Op:" /tmp/bench_output_$num_ops.txt | awk '{print $5}')
    local disk_mb=$(grep "Disk Usage:" /tmp/bench_output_$num_ops.txt | awk '{print $3}')
    
    # Determine pass/fail
    local status="✓"
    if [ $num_ops -ge 300000 ]; then
        # For 300K, must be under 4 seconds
        if (( $(echo "$time_sec > 4.0" | bc -l) )); then
            status="✗"
        fi
    elif [ $num_ops -ge 200000 ]; then
        # For 200K, should be under 3.5 seconds (proportional target)
        if (( $(echo "$time_sec > 3.5" | bc -l) )); then
            status="⚠"
        fi
    fi
    
    # Append to results
    echo "| $label | $num_ops | ${time_sec}s | ${avg_us}µs | ${disk_mb} MiB | $status |" >> "$OUTPUT_FILE"
    
    # Save detailed output
    cat /tmp/bench_output_$num_ops.txt >> workspace/carlos/benchmark_${num_ops}.log
    
    echo "  Time: ${time_sec}s, Avg: ${avg_us}µs/op, Disk: ${disk_mb}MiB [$status]"
}

# Create workspace for Carlos
mkdir -p workspace/carlos

# Create results table header
cat >> "$OUTPUT_FILE" << 'EOF'

### Baseline Performance (Before Cache Implementation)

| Test | Operations | Total Time | Avg Time/Op | Disk Usage | Status |
|------|-----------|------------|-------------|------------|--------|
EOF

# Run benchmarks at different scales
run_benchmark 100000 "100K Operations"
run_benchmark 200000 "200K Operations"
run_benchmark 300000 "300K Operations"

# Add analysis section
cat >> "$OUTPUT_FILE" << 'EOF'

**Legend**: ✓ = Pass, ⚠ = Warning (close to limit), ✗ = Fail

### Performance Analysis

EOF

# Calculate scaling factor
echo "Analyzing results..."
time_100k=$(grep "100K Operations" "$OUTPUT_FILE" | awk '{print $4}' | sed 's/s//')
time_300k=$(grep "300K Operations" "$OUTPUT_FILE" | awk '{print $4}' | sed 's/s//')

if [ -n "$time_100k" ] && [ -n "$time_300k" ]; then
    scaling=$(echo "scale=2; $time_300k / $time_100k / 3" | bc)
    
    cat >> "$OUTPUT_FILE" << EOF

**Scaling Factor**: Operations scale by ${scaling}x (1.0 = perfect linear)

The implementation shows $(if (( $(echo "$scaling < 1.1" | bc -l) )); then echo "excellent"; elif (( $(echo "$scaling < 1.3" | bc -l) )); then echo "good"; else echo "poor"; fi) scaling characteristics.

EOF
fi

# Add bottleneck analysis
cat >> "$OUTPUT_FILE" << 'EOF'

### Bottleneck Analysis

Based on the performance characteristics, the primary bottlenecks are likely:

1. **Disk I/O**: Each operation appears to require disk access with no caching
2. **Serialization**: Node serialization/deserialization overhead
3. **Seek time**: Random access pattern causes disk seeks

### Recommended Optimizations

To achieve the 4-second target for 300K operations:

1. **LRU Cache** (Priority: CRITICAL)
   - Cache frequently accessed nodes in memory
   - Target: 500-1000 nodes (~2-4 MB)
   - Expected improvement: 60-80% reduction in disk I/O
   
2. **Write Buffering** (Priority: HIGH)
   - Defer writes to reduce disk operations
   - Batch multiple updates when possible
   
3. **Read-ahead** (Priority: MEDIUM)
   - Prefetch child nodes during tree traversal
   - Reduce latency for sequential operations

---

## Reproducibility

To reproduce these benchmarks:

```bash
# Build the code
make clean && make

# Compile stress test
g++ -std=c++17 -O2 stress_test.cpp -o stress_test

# Run benchmarks
./run_benchmarks.sh ./code performance_results.md
```

All tests use seed=42 for deterministic results.

---

## Next Steps

1. **Before Cache Implementation**: Document these baseline results
2. **Implement LRU Cache**: Add caching layer to file_manager
3. **After Cache Implementation**: Re-run benchmarks and compare
4. **Target Improvement**: Achieve ≤4 seconds for 300K operations

---

**Benchmark completed**: $(date +"%Y-%m-%d %H:%M:%S")

EOF

echo ""
echo "Benchmark suite complete!"
echo "Results written to: $OUTPUT_FILE"
echo ""
echo "Summary:"
grep "Operations" "$OUTPUT_FILE" | grep -E "100K|200K|300K"

# Clean up temporary files
rm -f /tmp/bench_output_*.txt
rm -f data_file

echo ""
echo "Detailed logs saved to workspace/carlos/"
