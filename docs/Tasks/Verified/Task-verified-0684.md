# Verified Task — T-0684

**Sprint:** [SP-229] · **Epic:** none · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-20** (user, 2026-11-20: *"All steps pass on all platforms."*)
**Archived:** 2026-11-20.

---

## ✅ T-0684 — `[Cross]` The app's name is too close to Scrivener's

**Epic:** ⚠️ None yet · **Status:** ✅ Verified (user, 2026-11-20: *"All steps pass on all platforms."*) · **Sprint:** [SP-229] (✅ Closed) → [`../../Sprints/Closed/Sprint-SP-229.md`](../../Sprints/Closed/Sprint-SP-229.md) · **Date Requested:** 2026-11-17 (user, planning
[SP-220]: *"the name for my app is alarmingly similar to Scrivener.  We should address that at some point"*; filed (user, 2026-11-17: *"Q3a and Q7a as recommended, yes file the name Task"*))

**What:** Decide whether Scrivi keeps its name. Scrivener (Literature & Latte) is the best-known app in the same category, and
SP-220 now imports its projects, so the two names will sit side by side in menus ("Import ▸ Scrivener Project…"). Questions for
its planning: whether to rename, before or after the alpha spreads; the name's reach (bundle IDs, the `.scrivi` / `.scrivworld`
extensions, `scrivi_*` ABI, schema IDs `scrivi.*.v1`, repo and docs) — the internal identifiers need not follow a display
rename.

**Scope ruled (user, 2026-11-20):** ✅ **rename what people see** (app name, menus, About, Settings, window titles, App Store Connect
record, docs, the future `.deb` name) ✅ **and the file extensions** (`.scrivi`, `.scrivworld`, `.scrivarchive`) — ⛔ **no
compatibility with the old extensions:** *"I have no other testers and we can reset or rename the projects as we go."* ⛔ **Not
the identifiers:** bundle ID `com.caposoft.scrivi`, the Application Support folder, the `scrivi_*` ABI, schema IDs, repo and
file names stay. ✅ **The new name is Inkwyn** (user, 2026-11-20: *"I got it on the app store connect.  Inkwyn."*) — *ink* + *wynn* (ᚹ, U+16B9, the rune for "joy" that scribes wrote as the letter ƿ). ⏳ Whether ᚹ appears in the name ("Inkᚹ") or only as a mark, and the exact extensions, are for the Sprint's planning.


**Built ([SP-229], 2026-11-20):** Inkwyn everywhere text is seen, `.inkwyn` / `.inkwynworld` / `.inkwynarchive`, `Inkwyn.app`, the Wynn stanza in About. Details and the identifiers kept: the Sprint.
