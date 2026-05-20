---
name: feedback-xcode-pbxproj
description: User commits and creates PRs from Xcode — pbxproj must be updated whenever new source files are added
metadata:
  type: feedback
---

User commits and creates PRs from Xcode, not from the terminal or Claude. This means every new `.cpp` or `.hpp` file Claude adds must also be registered in `ScriviCore.xcodeproj/project.pbxproj` in the same step — before building and testing, not after.

**Why:** If the Xcode file tree is not updated, the user cannot see new files in Xcode's navigator, cannot stage them, and cannot commit or create PRs without dropping to the terminal. This breaks their preferred workflow entirely.

**How to apply:** After writing any new source file, immediately update `project.pbxproj` — add a PBXFileReference entry and add the UUID to the correct PBXGroup children array. Never skip this step, even when resuming from a summarized session.
