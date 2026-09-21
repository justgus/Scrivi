#pragma once

#include <QJsonObject>
#include <QString>

// InspectorLayoutStore — the inspector layout, THROUGH ScriviCore
// (EP-041 / SP-142, T-0537). Originally EP-035 / SP-126, T-0486.
//
// ## ⛔ THIS CLASS NO LONGER TOUCHES THE FILESYSTEM
//
// ⚠️ It used to read and write `inspector-layout.json` itself, because there was
// no `scrivi_*` endpoint for the schema. ✅ [SP-141] built one
// (`scrivi_get_inspector_layout` / `scrivi_put_inspector_layout`) and converted
// Apple; ✅ T-0537 converts Linux, so ⛔ `scrivi.inspector-layout.v1` now has
// exactly ONE owner.
//
// ⚠️ **THE DUPLICATION WAS NOT THEORETICAL.** [I-0215]: this class preserved keys
// it did not understand and Apple's Swift store DROPPED them — same file, same
// schema, two behaviours. ✅ Linux was the one that had it right.
// ⛔ **DO NOT REINTRODUCE `QFile`/`QSaveFile` HERE.**
// ⚠️ ENFORCED, NOT ADVISORY: `scripts/check-package-boundary.sh` (EP-041 / T-0541)
// fails CI on `QFile`/`QSaveFile` anywhere in `platforms/linux/src/` outside its
// reasoned allow-list.
//
// ## ✅ What this class still owns: MEANING
//
// The core owns atomicity, durability and repair; ⛔ it does NOT interpret the
// document and does not know what a tab is. ✅ So the app still owns:
//   • the DEFAULTS for a project that has no layout yet (Apple's ruled defaults);
//   • which tab strings this build recognises, and the degrade-to-Writing rule;
//   • what to do about an `unreadable` document.
//
// ## ⚠️ PATCH, NEVER RECONSTRUCT — still the rule
//
// ⚠️ Linux does not model `stackSort`, `defaultStacks` or per-scene stacks.
// ✅ This class keeps the WHOLE decoded document and mutates only the keys it
// understands, so everything else round-trips untouched — including keys a future
// Scrivi adds. ⚠️ The core reinforces this (it never interprets the document), but
// ⛔ it cannot save a caller that reconstructs one.
class ScriviBridge;

class InspectorLayoutStore
{
public:
    // Schema identifier as Apple writes it. A file carrying a different schema is
    // still round-tripped (we never delete what we do not understand), but its
    // `selectedTab` is not trusted.
    static constexpr const char* kSchemaID = "scrivi.inspector-layout.v1";

    // Point the store at a project and load its layout THROUGH THE CORE.
    //
    // ⚠️ `bridge` must outlive this store (the shell owns both).
    // ✅ An ABSENT or UNREADABLE document is NOT an error — it yields Apple's ruled
    // defaults, which is what a project created before this file existed should see.
    // ⛔ An unreadable one additionally leaves the damaged file untouched on disk:
    // the core does not overwrite it, and neither do we until the writer acts.
    void load(ScriviBridge* bridge, const QString& projectRootPath);

    // True when the last load found a document the core could not parse. The
    // layout shown is defaults; ⚠️ the writer's real layout may be recoverable by
    // hand, so a caller may wish to say so rather than silently proceed.
    [[nodiscard]] bool loadWasUnreadable() const { return unreadable_; }

    // The persisted tab selection, or "writing" when absent/unknown.
    //
    // ⚠️ PROJECT-level, and it does NOT follow the scene (Apple parity:
    // "the selection persists at PROJECT level and does NOT follow the scene —
    // switching scenes reloads the current tab's cards for the new scene").
    //
    // ⚠️ An UNRECOGNISED tab string degrades to the default rather than failing
    // the load: a newer Scrivi may have written a tab this build lacks, and
    // refusing to open the project over it would be far worse than showing
    // Writing.
    QString selectedTab() const;

    // Persist a new tab selection, patching the loaded document in place.
    // No-op when nothing is loaded or the value is unchanged.
    void setSelectedTab(const QString& tab);

private:
    // Hand the (patched) document to the core. ✅ ATOMICITY IS THE CORE'S JOB now —
    // it writes a temp and renames, the same discipline this class used to
    // implement itself. ⛔ Do not reintroduce a write here.
    bool save() const;

    ScriviBridge* bridge_ = nullptr;   // not owned
    QString     projectRootPath_;
    QJsonObject document_;  // ⚠️ THE WHOLE DOCUMENT — see "patch, never reconstruct"
    bool        loaded_ = false;
    bool        unreadable_ = false;
};
