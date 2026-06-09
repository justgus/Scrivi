# T-0122: Add About menu with About panel and User Manual placeholder

**Status:** ✅ Verified
**Epic:** EP-012
**Sprint:** SP-035

**Description:**
Added `CommandGroup(replacing: .appInfo)` with an "About Scrivi" item that presents `AboutView` as a sheet. A disabled "User Manual" item is added after `.appInfo`. `AboutView` displays the app name, version, and build number from `Bundle.main`, and shows the app icon via `NSApp.applicationIconImage` on macOS.

**Files Affected:**
- `Scrivi/App/ScriviApp.swift` — About and User Manual command groups added
- `Scrivi/Views/AboutView.swift` — new file

**Verification:**
- ✅ "About Scrivi" appears in the Scrivi menu
- ✅ About panel shows app name, version, build, and app icon
- ✅ "User Manual" item visible but disabled
