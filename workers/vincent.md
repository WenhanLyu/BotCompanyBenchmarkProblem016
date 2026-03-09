---
reports_to: apollo
role: Build & Test Verifier
model: mid
---

# Vincent - Build & Test Verifier

## Your Task
Verify that the code builds cleanly and the sample test passes after all 3 fixes.

## What to Check

1. **Clean build from scratch**
   ```bash
   make clean
   make code 2>&1 | tee build_log.txt
   ```
   - Verify no compilation errors
   - Verify no warnings
   - Verify executable `code` is created

2. **Sample test execution**
   ```bash
   rm -f database.db
   ./code < test_sample.txt > actual_output.txt 2> stderr.txt
   echo "Exit code: $?"
   ```
   - Verify exit code is 0
   - Verify no stderr output
   - Verify output matches expected: "2001 2012\nnull\nnull\n"

3. **Exact output verification**
   ```bash
   cat actual_output.txt
   od -c actual_output.txt
   ```
   - Verify byte-for-byte match with expected output
   - Check for any extra spaces, newlines, or formatting issues

## Output Requirements
Create a detailed report in your workspace:
- `build_test_verification.md` with all findings
- Include full build output
- Include test execution output
- Include od -c output for exact byte comparison
- Clear PASS/FAIL verdict

## Success Criteria
✅ Build completes with no errors or warnings
✅ Sample test runs with exit code 0
✅ Output exactly matches "2001 2012\nnull\nnull\n"
✅ No stderr output during normal execution
