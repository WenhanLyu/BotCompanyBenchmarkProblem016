#!/bin/bash

# Test Verification Script for Issue #12
# Verifies all existing tests still pass after cache implementation

echo "===== Test Verification Report ====="
echo "Testing executable: ./code"
echo "Date: $(date)"
echo ""

# Test 1: Sample Test
echo "=== Test 1: Sample Test (test_sample.txt) ==="
echo "Expected output:"
echo "2001 2012"
echo "null"
echo "null"
echo ""
echo "Actual output:"
./code < test_sample.txt
RESULT=$?
echo ""
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
echo ""

# Test 2: Edge Cases
echo "=== Test 2: Edge Cases (test_edge_cases.txt) ==="
./code < test_edge_cases.txt > workspace/nina/edge_output.txt
RESULT=$?
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
cat workspace/nina/edge_output.txt
echo ""

# Test 3: Persistence Test Part 1
echo "=== Test 3: Persistence Test Part 1 (test_persist1.txt) ==="
rm -f data.txt  # Clean slate
./code < test_persist1.txt > workspace/nina/persist1_output.txt
RESULT=$?
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
cat workspace/nina/persist1_output.txt
echo ""

# Test 4: Persistence Test Part 2
echo "=== Test 4: Persistence Test Part 2 (test_persist2.txt) ==="
./code < test_persist2.txt > workspace/nina/persist2_output.txt
RESULT=$?
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
cat workspace/nina/persist2_output.txt
echo ""

# Test 5: Quick Test (boundary values, long keys)
echo "=== Test 5: Quick Test (test_quick.txt) ==="
./code < test_quick.txt > workspace/nina/quick_output.txt
RESULT=$?
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
cat workspace/nina/quick_output.txt
echo ""

# Test 6: Simple tiny test
echo "=== Test 6: Tiny Test (test_tiny.txt) ==="
./code < test_tiny.txt > workspace/nina/tiny_output.txt
RESULT=$?
echo "Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
    echo "✅ PASS"
else
    echo "❌ FAIL"
fi
cat workspace/nina/tiny_output.txt
echo ""

echo "===== Test Verification Complete ====="
