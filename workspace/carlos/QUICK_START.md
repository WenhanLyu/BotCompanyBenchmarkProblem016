# Quick Start - Performance Testing

## Running the Stress Tests

### One-Line Commands

```bash
# Test 100K operations
./stress_test 100000 42 ./code

# Test 200K operations  
./stress_test 200000 42 ./code

# Test 300K operations
./stress_test 300000 42 ./code
```

### Expected Output

```
=== B+ Tree Performance Stress Test ===
Generating 300000 operations (seed: 42)...
Writing test file: stress_test_input.txt
Running benchmark with ./code...

=== Performance Benchmark Results ===
Operations: 300000
Execution Time: 1.80565 seconds
Avg Time per Op: 6.01884 µs
Disk Usage: 4.35938 MiB
Status: SUCCESS

=== Requirements Check ===
Time limit (≤4s for 300K): PASS ✓
Memory limit (≤60 MiB): PASS ✓

🎉 All performance targets met!
```

## Compiling from Source

If you need to rebuild:

```bash
# Compile stress test
g++ -std=c++17 -O2 -Wall -Wextra stress_test.cpp -o stress_test

# Rebuild main code
make clean && make
```

## Files Created

The stress test creates temporary files:
- `stress_test_input.txt` - Test operations (auto-cleaned)
- `database.db` - B+ Tree data file
- `.stress_time_output` - Timing data (auto-cleaned)

Clean between runs:
```bash
rm -f database.db stress_test_input.txt .stress_time_output
```

## What Gets Tested

Each run tests:
- **60%** INSERT operations
- **30%** FIND operations
- **10%** DELETE operations

With:
- Variable key lengths (8, 16, 32, 64 bytes)
- Duplicate key handling
- Non-existent entry operations
- Deterministic results (seed=42)

## Interpreting Results

### Success Criteria

✅ **PASS** if:
- Execution time ≤4 seconds for 300K ops
- Disk usage ≤60 MiB
- No errors or crashes

⚠️ **WARNING** if:
- Time is 4-5 seconds (close to limit)

✗ **FAIL** if:
- Time exceeds 5 seconds
- Memory/disk exceeds limits
- Program crashes

### Current Performance

As of latest test:
- 100K ops: **0.44s** ✅
- 200K ops: **0.89s** ✅  
- 300K ops: **1.81s** ✅

All well under limits!

## Troubleshooting

### "command not found: stress_test"
```bash
# Make executable
chmod +x stress_test

# Or recompile
g++ -std=c++17 -O2 stress_test.cpp -o stress_test
```

### "executable not found: ./code"
```bash
# Build the main program
make
```

### Different results than documented
```bash
# Ensure clean state
rm -f database.db

# Verify you're using seed=42
./stress_test 300000 42 ./code
```

## Advanced Usage

### Custom seed
```bash
./stress_test 300000 12345 ./code
```

### Custom executable path
```bash
./stress_test 300000 42 /path/to/executable
```

### Save output
```bash
./stress_test 300000 42 ./code > benchmark_results.txt 2>&1
```

---

**Quick Reference**: All tests use seed=42 for reproducibility.  
**Typical runtime**: 100K: <1s, 200K: <1s, 300K: ~2s
