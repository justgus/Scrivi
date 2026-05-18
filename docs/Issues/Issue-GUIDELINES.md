# Issue Guidelines

## Documentation Requirements

**CRITICAL: All Issue reports MUST be documented in Issue-Documentation.md**

When a new Issue is created:

1. **Create an Entry in Issue-Documentation.md**
   2. Add new section with format: `## I-XXXX: [Title]`
   3. Include all required fields (see template below)
   4. Maintain sequential numbering (I-0001, I-0002, etc.)

2. **Required Fields**
   2. **Status:** (🔴 Open / ⚠️ In Progress / ✅ Resolved - Verified)
   3. **Platform:** (iOS, macOS, visionOS, All platforms)
   4. **Component:** (Affected component/file)
   5. **Severity:** (Critical / High / Medium / Low)
   6. **Description:** Clear description of the issue
   7. **Expected Behavior:** What should happen
   8. **Actual Behavior:** What actually happens
   9. **Date Identified:** YYYY-MM-DD
   10. **Steps to Reproduce:** (if applicable)
   11. **Root Cause Analysis:** Technical explanation
   12. **Resolution:** Implementation details
   13. **Files Affected:** List of modified files
14. **Assigned to Sprint:** The Sprint the Issue was assigned to
   15. **Verification:** Testing steps and results

3. **Location**
   2. File: `/Scrivi/Documentation/Issues/Issue-Documentation.md`
   3. Append new Issue Entries at the end
   4. Maintain separator line (`---`) between entries

## Issue Entry Template

````markdown
## I-XXXX: [Title of issue]

**Status:** 🔴 Open
**Platform:** [iOS / macOS / visionOS / All platforms]
**Component:** [Component name]
**Severity:** [Critical / High / Medium / Low]
**Sprint:** [Sprint number / "Not Assigned"]

**Description:**
[Clear description of the problem]

**Expected Behavior:**
[What should happen]

**Actual Behavior:**
[What actually happens]

**Steps to Reproduce:**
1. [Step 1]
2. [Step 2]
3. [Step 3]

**Impact:**
- [Impact item 1]
- [Impact item 2]

**Date Identified:** YYYY-MM-DD

**Root Cause Analysis:**
[Technical explanation of why the issue occurs]

**Resolution:**

**Fix Date:** YYYY-MM-DD
**Verification Date:** YYYY-MM-DD

**Implementation:**

1. **[Change description 1]** (File.swift:line)
   - [Details]

2. **[Change description 2]** (File.swift:line)
   - [Details]

**Code Changes:**
```swift
// Code examples if relevant
````

**Result:**
✅ [Outcome 1]
✅ [Outcome 2]

**Files Affected:**
- `FileName.swift` - [Description of changes]

**Verification:**
- ✅ [Test 1]
- ✅ [Test 2]

**Related Issues:**
- [Related Issue numbers or issues]

---
\`\`\`

## Status Indicators

- 🔴 **Open** - Issue identified, in the Backlog, not assigned a Sprint
- ⚠️ **In Progress** - Actively being investigated, assigned to a Sprint
- 🟡 **Resolved - Not Verified** - Fixed but not validated by user
- ✅ **Resolved - Verified** - Fixed and verified by user

**Authorization Note:** Claude is authorized to mark Issues as "Resolved - Not Verified" (🟡) when implementation is complete, but Claude can only mark an Issue as "Resolved - Verified" (✅) after the user’s explicit direction.

## Best Practices

1. **Document as you work** - Don't wait until the end
2. **Include line numbers** - Reference specific file locations
3. **Add code examples** - Show before/after when relevant
4. **Verify on device** - Always test fixes on actual hardware when possible
5. **Update status** - Keep status current throughout the process
6. **Cross-reference** - Link related Issues together

## Severity Levels

- **Critical:** Crashes, data loss, complete feature failure
- **High:** Major functionality broken, significant user impact
- **Medium:** Feature partially broken, workarounds available
- **Low:** Minor issues, cosmetic problems, non-essential features

## File Organization and Batch Management

### Batch File Policy

**Verified Issues are organized into batch files of 10 Issues each in the `Verified/` subfolder:**

- **Verified/DR-verified-0001-0010.md** - DRs 1-10
- **Verified/DR-verified-0011-0020.md** - DRs 11-20
- **Verified/DR-verified-0021-0030.md** - DRs 21-30
- **Verified/DR-verified-0031-0040.md** - DRs 31-40
- *...and so on*

**Rationale:** Keeps individual files manageable (\~800-1000 lines each) for easier navigation, editing, and version control. Subfolder organization keeps the main DR-Reports directory clean.

### When to Create a New Batch File

When Issue-00X0 (the 10th Issue in a batch) is verified:
1. Create new batch file in `Verified/` subfolder for next 10 Issues
2. Update Issue-Documentation.md index to reference new batch
3. Move "Unverified Issues" section to the new batch file

**Example:** When I-0030 is verified, create `Verified/Issue-verified-0031-0040.md` for the next batch.

### Quick Reference Index

**Issue-Documentation.md serves as a lean index with these sections:**
1. **Unverified Issues** - Table of active issues (naturally bounded)
2. **Verified Issues** - Table of batch files (one row per batch, grows slowly)
3. **Closed Issues** - Table of closed-without-verification batches
4. **Backlog Issues** - Reference to deferred issues
5. **Statistics** - Current counts only (no history log)

**MUST be updated whenever:**
- A new Issue is created (add to Unverified table)
- An Issue status changes (update status emoji in table)
- An Issue is verified (remove from Unverified table, update batch row status)
- Statistics counts change

**CRITICAL:** Do NOT add activity logs, summaries, or per-Issue detail tables to Issue-Documentation.md. Details belong in Issue-active.md (for active work) or the verified batch files (for completed work). The index must stay compact.

## File Naming Convention

**Main DR-Reports directory (`Documentation/Issues/`):**
- **Issue-Documentation.md** - Quick reference index (all DRs, always up-to-date).
- **Issue-active.md** - Active issues awaiting resolution or verification.  Created when necessary
- **Issue-GUIDELINES.md** - This file (documentation standards)
- **Issue-backlog-XXXX-YYYY.md** - Archived open/deferred issues

**Issue-verified subfolder (`Documentation/Issues/Verified/`):**
- **Issue-verified-XXXX-YYYY.md** - Verified issues in batches of 10

**Issue-closed subfolder (`Documentation/Issues/Closed/`):**
- **Issue-closed-XXX-XXX.md** - Closed issues (not verified, will not be verified)

### File Purpose Details

**Issues-backlog.md:**
- Contains only active Issues not assigned to a sprint.  Newly created Issues automatically are created in this file.  
	**Issues-active.md:**
- Contains only active Issues (In Progress, or Resolved-Not-Verified) assigned to a Sprint
- Updated frequently as work progresses
- Should never contain "Closed" or "Verified" Issues
- Located in main Issues directory

**Verified/Issue-verified-XXXX-YYYY.md:**
- Contains Issues that have been resolved and verified
- Organized in batches of 10
- Located in `Verified/` subfolder
- Once a Issue is here, it's considered closed, complete and successful

**Closed/Issue-closed-XXX-XXX.md:**
- Contains Issues that were closed WITHOUT verification
- Located in `Closed/` subfolder
- Used for:
	  - External limitations (e.g., AI provider safety filters)
	  - Issues superseded by other work (e.g., "Will be addressed by ER-XXXX")
	  - Design decisions to not fix
	  - Known issues with documented workarounds
- These Issues will NEVER be verified but should be preserved for historical reference
- Organized as separate files one for each closed Issue.  

**Issue-backlog.md:**
- Contains older open or deferred Issues not assigned a Sprint
- Located in main Issue-Reports directory
- Issues that remain open but are not actively being worked on
- Preserved for historical tracking/future work

---

## Issue-Documentation.md Update Checklist

**Use this checklist for EVERY Issue operation:**

### When Creating a New Issue:
- [ ] Issue added to Issue-backlog.md
- [ ] Issue added to Unverified table in Issue-Documentation.md
- [ ] Unverified count updated
- [ ] "Next available Issue” incremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Assigning an Issue to a Sprint[]()
- [ ] Issue removed from Issue-backlog.md
- [ ] Issue added to Issue-active.md
- [ ] Issue added to the Active Issues table
- [ ] Statistics updated
- [ ] Sprint assigned field updated
- [ ] "Last Updated" date updated

### When Moving an issue to the Backlog[]()
- [ ] Issue removed from Issue-active.md
- [ ] Issue removed from the Active Issues table
- [ ] Issue added to Issue-backlog.md
- [ ] Statistics updated
- [ ] Sprint assigned field set to “Not Assigned”
- [ ] "Last Updated" date updated

### When Resolving a Issue:
- [ ] Status changed to 🟡 in Issue-active.md
- [ ] Status emoji updated in Active table
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Verifying an Issue:
- [ ] Issue moved from Issue-unverified.md to appropriate batch file in `Verified/` subfolder
- [ ] Issue removed from Unverified table in Issue-Documentation.md
- [ ] Batch row status updated in Verified table (if batch completion changed)
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Closing an Issue Without Verification:
- [ ] Issue moved from Issue-unverified.md to appropriate file in `Closed/` subfolder
- [ ] Issue removed from "Active Issues" table (if it was there)
- [ ] Issue added to "Closed Issues (Not Verified)" section in Issue-Documentation.md
- [ ] "Reason for Closure" documented in the closed Issue file
- [ ] Closed count incremented in Issue-Documentation.md
- [ ] Unverified count decremented (if applicable)
- [ ] "Last Updated" date updated

**Reasons for Closing Without Verification:**
- **External Limitation:** Issue is in third-party code/service (e.g., AI provider behavior)
- **Superseded:** Work will be done in different ER (e.g., "Will be addressed by ER-0021")
- **Design Decision:** Team/user decides not to fix
- **Known Issue:** Issue acknowledged with documented workaround, fix deferred indefinitely

**Important:** Closed Issues should still include full documentation (description, root cause, proposed solutions, workarounds) for future reference. They are closed due to circumstances, not lack of documentation.

---

**REMINDER:** Every Issue operation MUST update Issue-Documentation.md before marking work as complete. This is not optional!


