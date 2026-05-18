# Task (T) Guidelines

## Purpose

The Task (T) system tracks planned improvements, new features, and requirement changes for Scrivi. Unlike Issues(I) which address bugs and unintended behavior, Tasks manage intentional evolution of the system.

## Task Lifecycle

```
Backlog → Active → Implemented → Verified
   🔵         🟡            🟡           ✅
	+	→ Closed
			 [RED CIRCLE]
```

### Status Definitions

1. **🔵 Backlog** - Enhancement has been identified and documented, awaiting assignment to a Sprint
   2. Claude or User can create proposed Tasks
   3. Used when planning future work

1. **[Red Circle]() Closed** - Enhancement has been closed and will not be implemented.  
   2. Claude or User can only close Tasks upon User Direction
   3. Used when documenting proposed changes that will not be implemented.  

2. **🟡 Active** - Enhancement is actively being worked on they are assigned to a Sprint
   3. Claude marks when starting implementation

3. **🟡 Implemented - Not Verified** - Implementation complete, awaiting user verification
   2. **ONLY CLAUDE** can mark as implemented
   3. Indicates code is written and ready for testing
   4. Must include test steps for verification

4. **✅ Implemented - Verified** - User has tested and confirmed the enhancement works
   2. Claude marks a Task as Implemented - Verified **ONLY AT THE DIRECT INSTRUCTION OF THE USER**.  
   3. Final state for completed enhancements

## Task Workflow

### Phase 1: Analysis & Design
1. **Analyze Request** - Understand what needs to change and why
2. **Review Current Implementation** - Examine existing code and architecture
3. **Design Approach** - Plan the implementation strategy
4. **Identify Impact** - Determine which components will be affected

### Phase 2: Requirements
1. **Codify Requirements** - Document what the enhancement must accomplish
2. **Define Acceptance Criteria** - Specify how to verify success
3. **Consider Edge Cases** - Think through unusual scenarios
4. **Update Documentation** - Plan documentation changes

### Phase 3: Implementation
1. **Execute Changes** - Write the code
2. **Self-Test** - Verify basic functionality
3. **Update Related Code** - Fix any impacted areas
4. **Document Implementation** - Record what was done and why

### Phase 4: Verification
1. **Provide Test Steps** - Clear instructions for user verification
2. **User Testing** - Developer tests the enhancement
3. **Verification** - Only user can mark as verified

## Task Template Structure

```markdown
## T-XXXX: [Brief Title]

**Status:** 🔵 Backlog / 🟡 Active / 🟡 Implemented - Not Verified / ✅ Implemented - Verified
**Component:** [Primary Component Name]
**Priority:** Critical / High / Medium / Low
**Date Requested:** YYYY-MM-DD
**Date Implemented:** YYYY-MM-DD (if applicable)
**Date Verified:** YYYY-MM-DD (if applicable)
**Sprint Asigned:** [Sprint Number] / "Not Assigned"

**Rationale:**
[Why this enhancement is needed - business case, user benefit, technical debt reduction]

**Current Behavior:**
[How the system currently works]

**Desired Behavior:**
[How the system should work after enhancement]

**Requirements:**
1. [Specific requirement 1]
2. [Specific requirement 2]
3. [Specific requirement 3]

**Design Approach:**
[High-level implementation strategy]

**Components Affected:**
- Component 1: [What changes]
- Component 2: [What changes]

**Implementation Details:**
[Detailed description of changes made - filled in during implementation]

**Test Steps:**
1. [Step to verify requirement 1]
2. [Step to verify requirement 2]
3. [Expected results]

**Notes:**
[Any additional context, trade-offs, or future considerations]
```

## Authorization Rules

### What Claude CAN Do:
- ✅ Create backlog Tasks (🔵 Backlog)
- ✅ Update Task status to Active (🟡 Active)
- ✅ Mark Tasks as Implemented - Not Verified (🟡 Implemented - Not Verified)
- ✅ Add implementation details and test steps
- ✅ Update Task documentation and tracking files

### What Claude CANNOT Do:
- ❌ Mark Tasks as Verified (✅ Implemented - Verified) without direct user aproval
- ❌ Skip the analysis and design phases
- ❌ Implement without documenting requirements
- ❌ Close Tasks without direct user approval

## File Organization and Workflow Management

**Location:** All Task files are now located in `Documentation/Tasks/` (separate from Issues)

### Active Work Files

**Tasks are organized into active work files based on status in the main Task-Reports directory:**

- **Task-backlog.md** - 🔵 Proposed enhancements awaiting assignment to a Sprint
- **Task-active.md** - 🟡 Taskss actively being implemented (assigned to a Sprint)
- **Task-unverified.md** - 🟡 Implemented and awaiting user verification

**Rationale:** This structure mirrors Agile workflow boards (Backlog → Active → Done) and makes it easy to see what's being worked on vs. what's planned. It also reduces file size for easier processing by Claude Code.

### Backlog Management (Optional)

For long-term or lowerpriority Tasks

- **Task-backlog.md** - Proposed enhancements waiting assignment to a Sprint

**Backlog Tasks** are similar to Agile product backlog items—documented for future consideration but not currently in active development. Move Tasks from backlog to active when assigned to a Sprint.

### Verified Tasks (Archive)

**Verified Tasks are organized into files with flexible sizing in the `Tasks/Verified/` subfolder:**

- **Tasks/Verified/Task-verified-0001.md** - Task T-0001
- **Tasks/Verified/Task-verified-0002.md** - Task T-0002
- **Tasks/Verified/Task-verified-0003.md** - Task T-0003
- *...and so on*

**Rationale:** Unlike Issues which use fixed batches of 10, Tasks use individual files for each enhancement since enhancements are typically more detailed and ideally occur less frequently. Subfolder organization keeps the main Tasks directory clean.

### Quick Reference Index

**Task-Documentation.md serves as a lean index with three sections:**
1. **Backlog Taskss** - Table of proposed enhancements (from Task-backlog.md)
2. **Active Taskss** - Table of active work (from Task-active.md)
3. **Complete Unverified Taskss** - Table of implemented but unverified (from Task-active.md)
4. **Verified ERs** - Table of ER files (one row per batch, grows slowly)
5. **Statistics** - Current counts only (no history log)

**MUST be updated whenever:**
- A new Task is created (add to Backlog table)
- An Task status changes to Active(move from Backlog to Active table)
- An Task is implemented (move from In Active to Complete Unverified table)
- An Task is verified (remove from Complete Unverified table, add batch row)
- Statistics counts change

**CRITICAL:** Do NOT add activity logs, summaries, or per-Task detail tables to Task-Documentation.md. Details belong in the active work files (Task-backlog.md, Task-active.md, Task-unverified.md) or the verified files. The index must stay compact.

## File Naming Convention

**Main Task-Reports directory (`Documentation/Tasks/`):**
- **Task-Documentation.md** - Quick reference index (all ERs, always up-to-date)
- **Task-backlog.md** - 🔵 Proposed enhancements (awaiting Sprint assignment)
- **Task-active.md** - 🟡 Active development (In Progress)
- **Task-unverified.md** - 🟡 Implemented but not yet verified by user
- **Task-Guidelines.md** - This file (documentation standards)

**Task-verified subfolder (`Documentation/Tasks/Verified/`):**
- **Task-verified-XXXX.md** - Verified enhancements in sequential batches

**Note:** Issues (I) are in a separate folder structure at `Documentation/Issues/` (see Issues/Issue-GUIDELINES.md)

## Best Practices

1. **One Enhancement, One Task** - Don't bundle multiple unrelated enhancements
2. **Clear Requirements** - Be specific about what success looks like
3. **Document Design Decisions** - Explain why you chose this approach
4. **Consider Alternatives** - Note other approaches considered and why they were rejected
5. **Update as You Go** - Keep the ER updated during implementation
6. **Thorough Test Steps** - Make verification easy for the user

## Relationship to Issue System

- **Issues** fix bugs and unintended behavior
- **Tasks** add features and change requirements
- Both systems use similar workflows and documentation
- Both require user verification before marking as complete
- Use cross-references when a Task addresses technical debt identified in a Issue

## Priority Levels

- **Critical** - Blocking user workflow or needed for upcoming release
- **High** - Important improvement with significant user benefit
- **Medium** - Useful enhancement, can be scheduled flexibly
- **Low** - Nice to have, implement when time allows

---

## Task-Documentation.md Update Checklist

**Use this checklist for EVERY Task operation:**

### When Creating a New Task:
- [ ] Task added to Task-backlog.md
- [ ] Task added to Backlog table in Task-Documentation.md
- [ ] Backlog count updated
- [ ] "Next available Task” incremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Starting Implementation (Backlog → Active):
- [ ] Task moved from Task-backlog.md to Task-active.md
- [ ] Status changed to 🟡 Active
- [ ] Task moved from Backlog table to Active table in Task-Documentation.md
- [ ] Backlog count decremented, Active count incremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Marking Task as Implemented - Not Verified:
- [ ] Task moved from Task-active.md to Task-unverified.md
- [ ] Status changed to 🟡 Implemented - Not Verified
- [ ] Task moved from Active table to Unverified table in Task-Documentation.md
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When User Verifies a Task:
- [ ] Task moved from Task-unverified.md to appropriate file in `Verified/` subfolder
- [ ] Status changed to ✅ Implemented - Verified
- [ ] Task removed from Unverified table in Task-Documentation.md
- [ ] Row added to Verified table
- [ ] Unverified count decremented, Verified count incremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Moving Task to Backlog (Optional):
- [ ] ER moved from Task-active.md to Task-backlog.md
- [ ] ER removed from Active table in Task-Documentation.md
- [ ] ER added to Backlog table in Task-Documentation.md
- [ ] Note added to Task explaining backlog reason
- [ ] Active count decremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

### When Moving a Task to Closed (Optional):
- [ ] Task moved from Task-backlog.md to or Task-active.md to an appropriate file in `Closed/` subfolder
- [ ] Status changed to [Red Circle]() Closed
- [ ] Task removed from Backlog or Active table in Task-Documentation.md
- [ ] Row added to Closed table
- [ ] Backlog or Active count decremented, Closed count incremented
- [ ] Statistics updated
- [ ] "Last Updated" date updated

---

**REMINDER:** Every Task operation MUST update Task-Documentation.md before marking work as complete. This is not optional!

---

*Version: 2.0*
*Last Updated: 2026-01-10*

