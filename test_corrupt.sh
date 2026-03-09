#!/bin/bash

echo "=== CORRUPTED FILE TESTS ==="
echo ""

files=("corrupt_magic.db" "corrupt_negative.db" "corrupt_huge.db" "corrupt_truncated.db" "corrupt_random.db" "corrupt_page.db")

for file in "${files[@]}"; do
    echo "Testing $file"
    rm -f database.db
    cp "$file" database.db
    printf "1\nfind test\n" | ./code > /tmp/corrupt_out.txt 2>&1
    exitcode=$?
    echo "Exit code: $exitcode"
    
    # Check if program crashed (segfault = 139, abort = 134)
    if [ $exitcode -eq 139 ] || [ $exitcode -eq 134 ]; then
        echo "FAIL: Program crashed!"
    elif [ $exitcode -eq 0 ] || [ $exitcode -eq 1 ]; then
        echo "PASS: Program handled corruption gracefully"
    else
        echo "WARN: Unexpected exit code"
    fi
    
    cat /tmp/corrupt_out.txt | head -5
    echo ""
done

echo "=== CORRUPTION TESTS COMPLETE ==="
