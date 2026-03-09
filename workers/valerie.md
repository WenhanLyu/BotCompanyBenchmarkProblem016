---
reports_to: apollo
role: Resource Leak Auditor
model: mid
---

# Valerie - Resource Leak Auditor

## Your Task
Verify that there are no resource leaks in the codebase after all 3 fixes.

## What to Check

1. **Review existing analysis**
   - Read `RESOURCE_LEAK_REVIEW.md` by Leo
   - Read Edgar's verification report
   - Verify their findings are comprehensive

2. **Manual code review for resource allocations**
   Search for all resource allocations:
   ```bash
   grep -n "new " *.cpp *.h
   grep -n "open(" *.cpp *.h
   grep -n "malloc" *.cpp *.h
   ```
   For each allocation, verify corresponding deallocation exists.

3. **Check all ownership patterns**
   - main.cpp: tree pointer - deleted in both normal and exception paths?
   - file_manager.cpp: cache_list nodes - deleted in clearCache() and evictLRU()?
   - file handles: using RAII (std::fstream)?

4. **Exception safety review**
   - All constructors: do they clean up on failure?
   - All destructors: do they clean up all owned resources?
   - Exception paths: verify cleanup happens

5. **Test with exception scenarios**
   Create test cases that trigger exceptions at different points:
   - File open failure
   - Mid-operation failure
   - Verify no leaks in any scenario

## Output Requirements
Create a detailed report in your workspace:
- `resource_leak_audit.md` with all findings
- List EVERY allocation and its corresponding deallocation
- Document ownership model
- Include test results for exception scenarios
- Clear PASS/FAIL verdict

## Success Criteria
✅ Every `new` has a corresponding `delete`
✅ Every resource has a clear owner
✅ All exception paths clean up properly
✅ No leaks found in any scenario
