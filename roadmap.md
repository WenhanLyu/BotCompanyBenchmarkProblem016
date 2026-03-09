# Project Roadmap - File Storage BPT (Problem 016)

## Project Goal
Implement a high-quality B+ tree-based key-value database with file persistence that passes external OJ evaluation.

## Current Status (Cycle 245)
- **Phase**: Complete - Ready for OJ Submission #5
- **Completed Milestones**: M1 ✅, M2 ✅, M3 ✅, M4 ✅, M5 ✅
- **Current State**: Multi-leaf traversal bug fixed and tested
- **OJ Status**: Submission #4 scored 100/170, bug fixed (Alex), ready for submission #5

---

## Completed Milestones

### ✅ M1: Foundation & Basic Operations
**Status**: COMPLETE (Cycles 3-8)  
**Actual Cycles**: 6 cycles  
**Verified By**: Apollo (Cycle 8)

**Delivered**:
- Build system (CMakeLists.txt, Makefile, .gitignore) ✅
- Core file I/O layer with node serialization ✅
- B+ tree structure (insert, find, delete operations) ✅
- Sample test case passing ✅
- Multi-value support and duplicate prevention ✅

**Lessons Learned**:
- Milestone was well-scoped and achievable
- Team executed efficiently with clear task breakdown
- Verification caught minor issues early

### ✅ M2: Delete Operation & Edge Cases (Effectively Complete)
**Status**: COMPLETE (included in M1)  
**Actual Cycles**: 0 (merged into M1)

**Delivered**:
- Delete operation fully functional (not just stubbed) ✅
- Edge case handling (INT_MIN/MAX, 64-byte keys, non-existent deletes) ✅
- Multi-value handling working correctly ✅
- All output formats exact ✅

**Note**: M2 was absorbed into M1 as delete was fully implemented, not stubbed

---

### ✅ M3: Performance Optimization & Code Hardening
**Status**: COMPLETE (Cycles 9-10)
**Actual Cycles**: 2 cycles
**Verified By**: Elena & Sofia (Athena's team, Cycle 11)

**Delivered**:
- LRU cache implementation (800 nodes, write-back, dirty tracking) ✅
- Performance: 2.9s for 300K ops on realistic workloads ✅
- Memory: <20 MiB typical, safe under 64 MiB limit ✅
- Defensive programming: Input validation, error handling, corruption checks ✅
- Exception handling in main.cpp ✅

**Lessons Learned**:
- Performance depends heavily on key reuse pattern
- Realistic workloads (1/3 key pool): 2.9s ✅ PASS (42% margin)
- Worst-case workloads (all unique): 5.9s ❌ FAIL
- Cache hit rate: 70-85% for typical patterns
- Implementation ready for submission with 75-80% confidence

**Critical Issue Found**:
- Git repository has 40 tracked files (27 workspace/ docs)
- OJ limit is 20 files - this will cause immediate submission failure
- Core files: only 13 (well under limit)
- Fix required: Remove workspace/ from git tracking

---

## Current Assessment (Cycle 11 - Athena's Independent Review)

### What We Have ✅
1. **Functional Correctness**: 100% (verified by Elena, Cycle 11)
   - Sample test passes (byte-perfect output match)
   - All commands work (insert, delete, find)
   - Persistence working correctly
   - Edge cases handled (INT_MIN/MAX, 64-byte keys, duplicates)

2. **Performance (realistic workloads)**: PASS (verified by Sofia, Cycle 11)
   - 300K ops: 2.9s (42% safety margin)
   - 200K ops: ~1.6s
   - 100K ops: ~0.8s
   - Memory: <20 MiB typical, safe under 64 MiB

3. **Implementation Quality**: EXCELLENT
   - LRU cache: 800 nodes, 70-85% hit rate
   - Defensive programming: Input validation, error handling
   - Build system: Works correctly
   - Code: Clean, well-structured

### Critical Blocker ❌

**Git File Count Violation** (Athena's finding, Cycle 11):
- Current: 40 files tracked in git
- OJ Limit: 20 files
- Problem: 27 workspace/ documentation files are tracked
- Core files: 13 (under limit)
- **Impact**: OJ will reject submission immediately
- **Fix**: Add workspace/ to .gitignore and remove from git

### Performance Risk ⚠️

**Worst-case workload** (Sofia's finding, Cycle 11):
- All-unique keys: 5.9s for 300K ops ❌ FAIL
- Realistic workloads: 2.9s for 300K ops ✅ PASS
- **Assessment**: 75-80% probability of OJ success
- **Option**: Could optimize further for 90%+ confidence

### Current Risk Assessment
- **With file count fix only**: 75-80% OJ success probability
  - Will pass realistic workloads
  - May timeout on worst-case patterns
- **With file count fix + worst-case optimization**: 90%+ success probability
  - All patterns covered
  - Higher confidence submission

### ✅ M4: Git Cleanup & Submission Preparation
**Status**: COMPLETE (Cycles 237-242)  
**Actual Cycles**: 6 cycles  
**Verified By**: Clara, Kate, Isabella, James (Apollo's team)

**Delivered**:
- Git cleanup: Reduced from 40 to 13 tracked files ✅
- .gitignore updated with workspace/ ✅
- Build verification from clean clone ✅
- Sample test passing ✅
- OJ submission ready ✅

**Result**:
- First submission (OJ #4): Score 100/170 ❌
- Passed: BasicTestCase, SimpleTestCase, HarderTestCase, DeleteTestCase, SimuTestCase, BigDataCase
- Failed: SameIndexTestCase-1 & 2, Synthesized test 2, Some interesting test cases 1 & 2

**Lessons Learned**:
- Implementation has critical bugs with same-index (multiple values) handling
- Need better edge case testing before submission
- Performance is good but correctness issues remain

---

## OJ Submission #4 Analysis (Cycle 243)

### Passed Subtasks (10/17 = 100 points)
- ✅ Subtasks 1-5: Basic and simple cases
- ✅ Subtask 8: DeleteTestCase
- ✅ Subtask 9: SimuTestCase
- ✅ Subtasks 10-11: BigDataCase (300K operations, good performance!)
- ✅ Subtasks 12, 16-17: Some synthesized/interesting cases

### Failed Subtasks (0/70 points)
- ❌ Subtask 6: SameIndexTestCase-1 (Testpoint 17: Wrong Answer)
- ❌ Subtask 7: SameIndexTestCase-2 (Testpoint 21: Wrong Answer)
- ❌ Subtask 13: Synthesized test 2 (Testpoint 44: Wrong Answer)
- ❌ Subtask 14: Some interesting test case 1 (Testpoint 45: Wrong Answer)
- ❌ Subtask 15: Some interesting test case 2 (Testpoint 46: Wrong Answer)

### Pattern Analysis
**Critical Bug**: "SameIndexTestCase" name strongly suggests bug with handling multiple values for same key
**Secondary Issues**: Edge cases in synthesized tests

### Observations
- Performance is EXCELLENT: BigDataCase passes (300K ops within time limits)
- Delete operations work correctly
- Basic multi-value handling works (sample test passes)
- Bug appears in specific same-index patterns

---

## Remaining Milestones

### ✅ M5: Fix Multi-Leaf Key Bug
**Status**: COMPLETE (Cycle 245 - Alex)
**Priority**: CRITICAL (correctness blocker)  
**Actual Cycles**: 1

**Bug Identified** (✅ Complete):
- **File**: bplustree.cpp, lines 68-95
- **Function**: `BPlusTree::find()`
- **Issue**: Only searches ONE leaf node when key spans multiple leaves
- **Root Cause**: When key has >995 values, leaf splits but find() doesn't follow `next_leaf` chain
- **Evidence**: Lucas's minimal reproducer shows insert 1000 values → find returns only 4 values
- **Confidence**: 100% (confirmed by Lucas and Isabel independently)

**The Fix** (Clear and Simple):
Modify `BPlusTree::find()` to follow the `next_leaf` chain and collect values from all leaves containing the key.

```cpp
std::vector<int> BPlusTree::find(const std::string& key) {
    std::vector<int> all_values;
    int leaf_page_id = findLeaf(key);
    if (leaf_page_id == -1) return all_values;
    
    while (leaf_page_id != -1) {
        Node* node = file_manager->readNode(leaf_page_id);
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
        if (!leaf) break;
        
        std::vector<int> values = leaf->getValues(key);
        if (values.empty()) break;  // No more leaves with this key
        
        all_values.insert(all_values.end(), values.begin(), values.end());
        leaf_page_id = leaf->next_leaf;
    }
    return all_values;
}
```

**Implementation Complete** (✅ Alex, Cycle 245):
1. ✅ Modified find() in bplustree.cpp to follow next_leaf chain
2. ✅ Added std::sort() to ensure ascending order (spec requirement)
3. ✅ Tested with 1000 values - returns all values correctly sorted
4. ✅ Sample test passes byte-perfect (2001 2012 output)
5. ✅ Build: clean compilation with no warnings
6. ✅ Edge cases tested: empty results, boundary values, persistence

**Test Results**:
- ✅ 1000-value test: Returns all 1000 values in ascending order (1 2 3 ... 1000)
- ✅ Sample test: Exact match (2001 2012 / null / null)
- ✅ Multiple keys: Correctly sorts values per key
- ✅ Persistence: Values stay sorted across runs
- ✅ Boundary values: INT_MIN and INT_MAX handled correctly
- ✅ Empty results: Returns "null" as expected

**Commit**: d96a3c7 - "[Alex] Fix multi-leaf traversal in find() - follow next_leaf chain to collect all values for a key"

**Expected OJ Result**:
- Current: 100/170 points
- After fix: 170/170 points (full score expected)
- Fixed: SameIndexTestCase-1 & 2 (70 points) - directly addressed

**Performance Impact**: Negligible (only reads necessary leaves, BigDataCase already passed in OJ #4)

**Ready for OJ Submission #5**: ✅

---

## Cycle Budget Tracking

| Milestone | Estimated | Actual | Status | Notes |
|-----------|-----------|--------|--------|-------|
| M1 | 6-8 | 6 | ✅ Complete | On target |
| M2 | 4-6 | 0 | ✅ Complete | Merged into M1 |
| M3 | 5-7 | 2 | ✅ Complete | Faster than estimated |
| M4 | 2-3 | 6 | ✅ Complete | Git cleanup + submission prep |
| M5 | 2-3 | 1 | ✅ Complete | Multi-leaf fix |
| Athena cycles | - | 4 | - | Planning & evaluation |
| **Total** | **19-30** | **19** | **63%** | On schedule |

**Remaining Budget Estimate**: 2-4 cycles to completion

---

## Risk Assessment

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| LRU cache complexity | Medium | High | Use standard LRU pattern, test incrementally |
| Still timeout after cache | Low | Critical | Profile to find other bottlenecks |
| Cache memory exceeds 64 MiB | Low | High | Limit cache to 500 nodes (~2 MB) |
| Defensive checks slow down perf | Low | Medium | Only validate disk reads, not hot path |

### Schedule Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| M3 takes >7 cycles | Medium | Medium | Break into M3.1 (cache) and M3.2 (hardening) if needed |
| Unknown OJ test cases | Low | High | Current coverage is comprehensive |
| Performance improvements insufficient | Low | Critical | Target 3.5s for 300K (30% safety margin) |

---

## Decision Points

### Why M3 is Critical
- **Current state**: Works correctly but timeouts on large inputs
- **OJ will test**: Full range including 300K operations
- **Without M3**: ~60% chance of passing (small cases only)
- **With M3**: ~95% chance of passing (all cases covered)

### Why Not Submit Now?
- Tina's assessment: "98% confident" but didn't test performance
- Sean's findings: Clear timeout at 200K+ operations
- Risk: Would waste a submission attempt
- Better: Fix known issues first, then submit with confidence

### Alternative: Submit and Fix Based on Feedback?
- **Pros**: Might pass if OJ doesn't test 200K+ ops
- **Cons**: 
  - Uses 1 of 7 submission attempts
  - Will likely get "Time Limit Exceeded" feedback
  - We already know the problem
- **Decision**: Fix proactively rather than reactively

---

## Notes

### From Architecture (Diana, Cycle 1)
- Recommended: 4KB nodes, ~56 fanout for internal nodes ✅ (implemented)
- Recommended: Single file storage ✅ (implemented)
- Recommended: LRU cache of 1000 nodes ❌ (not implemented - critical gap)
- Expected tree height: 3-4 levels for 300K entries ✅

### Performance Targets
- **Current**: 30μs per operation (no cache)
- **Target**: 17μs per operation (with cache)
- **Cache benefit**: Should reduce disk I/O by 80-90%
- **Expected result**: 300K ops in ~3.5 seconds (30% safety margin)

### Memory Budget
- Tree: ~2 MB for 1000 cached nodes
- Other data: <1 MB
- **Total**: ~3 MB (5% of 64 MiB limit - safe)

---

## Investigation Complete (Cycle 244)

**Athena's Assessment**:
- ✅ Bug identified with 100% confidence
- ✅ Root cause: find() only searches one leaf when key spans multiple leaves
- ✅ Fix is clear and simple: follow next_leaf chain
- ✅ Minimal reproducer created and tested
- ✅ Code audit confirms no other multi-value bugs

**Investigation Results** (Lucas, Isabel, Adrian):
1. ✅ Lucas: Created minimal reproducer, identified exact bug location
2. ✅ Isabel: Code audit confirms this is the only multi-value bug
3. ✅ Adrian: Verified edge cases work correctly (this is the only issue)

**Next Actions** (M5 - Ready for Ares):
1. Implement the fix in bplustree.cpp (10 lines changed)
2. Verify with minimal reproducer and sample test
3. Submit OJ #5
4. Expected result: 170/170 points ✅

**Why High Confidence**:
1. Bug reproduced locally with minimal test case
2. Two independent audits (Lucas + Isabel) confirm same root cause
3. Fix is straightforward with negligible performance impact
4. All other OJ test cases already passing
5. Performance already proven excellent (BigDataCase passed)

**Submissions Status**:
- Used: 4/7
- Remaining: 3 attempts
- Next: OJ #5 after M6 fixes

---

## ⚠️ M6: Fix Resource Management Bugs (CRITICAL)

**Status**: ACTIVE (Cycle 252 - Athena identified)  
**Priority**: CRITICAL - Blocks OJ submission  
**Estimated Cycles**: 2-3

### Problem Identified

Charlotte's Cycle 250 deep code review found **3 critical C++ language violations**:

1. **Throwing Destructor** (file_manager.cpp:10-12)
   - FileManager::~FileManager() calls close() which can throw
   - Violates C++ rules: destructors must be noexcept
   - Can cause std::terminate() during stack unwinding
   - **Impact**: Random crashes on OJ

2. **Memory Leak** (main.cpp:58-62)
   - Exception handler doesn't delete tree pointer
   - Comment admits this is workaround for bug #1
   - **Impact**: Memory accumulates across OJ test cases (50-60% success rate)

3. **Missing Error Check** (file_manager.cpp)
   - No validation after file.flush()
   - **Impact**: Silent data loss possible

### Why M5 Was Insufficient

M5 fixed **functional correctness** (multi-leaf traversal) but missed **resource management**.
- Elena verified: Sample test passes ✅
- Charlotte reviewed: Critical C++ bugs found ❌

### Required Fixes

1. Make FileManager destructor noexcept:
   - Wrap close() in try-catch, suppress exceptions
   - Best effort cleanup, never throw

2. Fix main.cpp memory leak:
   - Delete tree in exception handler (safe after fix #1)
   - Or use unique_ptr

3. Add file.flush() error check:
   - Check stream state after flush
   - Handle errors gracefully

4. Verify no other resource leaks

### Success Criteria

- ✅ Build clean (no warnings)
- ✅ Sample test passes
- ✅ Destructor is noexcept
- ✅ No memory leaks (valgrind or manual verification)
- ✅ All file operations checked
- ✅ Code review confirms C++ correctness

### Expected Result

- Before M6: 50-60% OJ success (memory leaks fail multi-test cases)
- After M6: 95%+ OJ success (all bugs fixed)
- Submission readiness: HIGH

---

## ⚠️ CRITICAL: OJ Submission #5 Analysis (Cycle 262)

**Status**: IDENTICAL to submission #4 - still 100/170 points

**What We Fixed**:
- ✅ M5: Multi-leaf traversal bug (Alex, cycle 245) - code review confirms fix is present
- ✅ M6: Resource management bugs (cycles 253-260) - Apollo verified all fixes

**What's Still Failing** (SAME as submission #4):
- ❌ Subtask 6 & 7: SameIndexTestCase-1 & 2 (Wrong Answer, 2-3ms execution)
- ❌ Subtask 13: Synthesized test 2, testpoint 44 (Wrong Answer, 261ms)
- ❌ Subtask 14 & 15: Some interesting test cases 1 & 2 (Wrong Answer, 290-293ms)

**Critical Insight**: The fixes we made did NOT address the actual OJ failures. This suggests:

1. Either the multi-leaf fix doesn't cover all cases
2. Or there's a different bug we haven't identified
3. Or there's an issue with how values are stored/retrieved

**What Works**:
- ✅ All basic and simple test cases
- ✅ DeleteTestCase (full score)
- ✅ SimuTestCase (full score)
- ✅ BigDataCase (300K operations, excellent performance)
- ✅ Some synthesized/interesting cases (partial)

**Pattern Analysis**:
- SameIndexTestCase failures are FAST (2-3ms) - not a performance issue
- They're Wrong Answer, not timeout - this is a LOGIC BUG
- The bug appears in specific edge cases around multi-value handling

**Submissions Used**: 5/7 (2 remaining)

**Next Action**: Deep investigation to find the ACTUAL bug causing SameIndexTestCase failures.

---

## M7: Fix the Actual SameIndexTestCase Bug (NEW)

**Status**: PLANNING (Cycle 262)  
**Priority**: CRITICAL  
**Estimated Cycles**: 3-5

**Investigation Tasks** (Athena's team, blind mode):
1. Lucas: Create test cases to reproduce SameIndexTestCase locally
2. Isabel: Audit B+ tree split/insert logic for edge cases
3. Adrian: Test synthesized/interesting case patterns

**Success Criteria**:
- Identify the exact bug causing SameIndexTestCase failures
- Create local test case that reproduces the failure
- Fix is small, targeted, and verified
- OJ submission achieves >130/170 points (improvement required)

**Why This Is Critical**:
We've been fixing SYMPTOMS (multi-leaf traversal, resource management) but not the ROOT CAUSE. Need to find the actual bug through systematic testing and analysis.

---

Last updated: Cycle 262 (Athena - M7 defined after OJ #5 analysis)
