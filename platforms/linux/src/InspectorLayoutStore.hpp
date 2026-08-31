#pragma once

#include <QJsonObject>
#include <QString>

// InspectorLayoutStore — reads and writes `inspector-layout.json` at a project
// root (EP-035 / SP-126, T-0486). The Qt counterpart of Apple's
// Scrivi/App/InspectorLayoutStore.swift.
//
// ## ⚠️ This schema is SHARED, ALREADY EXISTS, and is APP-SIDE
//
// `scrivi.inspector-layout.v1` sits at the project root. ⚠️ **There is NO
// `scrivi_*` endpoint for it** — `SceneMetaJson.hpp:53` calls it "view
// configuration" and deliberately keeps it out of scene metadata, so Apple reads
// and writes the file directly and so must Linux.
//
// ⚠️ **Apple already populates it in real projects.** A writer's project carries
// `selectedTab`, `inspectorHidden`, `defaultStacks`, `stackSort` and a per-scene
// `scenes` map before Linux ever opens it.
//
// ## ⚠️ PATCH, NEVER RECONSTRUCT — the whole point of this class
//
// ⚠️ **Linux does not model `stackSort`, `defaultStacks` or per-scene stacks in
// this sprint.** Building a typed struct and serialising it back would therefore
// DELETE every one of those keys.
//
// ⚠️ **The loss would be silent and invisible until the writer reopened the
// project on the Mac** and found her card layout gone. That is the T-0436/T-0437
// "patch, never reconstruct" rule (Porting Outline §4.3) applied to view
// configuration rather than to an object — the failure mode is identical.
//
// ✅ **So this class keeps the WHOLE decoded document** and mutates only the keys
// it understands. Everything else round-trips untouched, including keys a future
// Scrivi adds that this build has never heard of.
//
// ## ⚠️ Accepted risk, recorded deliberately
//
// ⚠️ **Two app-side implementations of one schema, with no core to arbitrate.**
// ⚠️ **If a THIRD platform needs this file, move the schema into ScriviCore
// rather than writing a third parser** — that is the point at which the
// duplication stops being affordable.
class InspectorLayoutStore
{
public:
    // Schema identifier as Apple writes it. A file carrying a different schema is
    // still round-tripped (we never delete what we do not understand), but its
    // `selectedTab` is not trusted.
    static constexpr const char* kSchemaID = "scrivi.inspector-layout.v1";

    // Point the store at a project root and load its layout. A missing or
    // unreadable file is NOT an error — it yields Apple's ruled defaults, which
    // is what a project created before this file existed should see.
    void load(const QString& projectRootPath);

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
    // Write the (patched) document back atomically. Called by the setters.
    //
    // ⚠️ Writes via a temporary + rename so an interrupted write cannot leave a
    // truncated layout file — losing the file degrades gracefully, but a HALF
    // file would not parse and would take the writer's Apple-side layout with it.
    bool save() const;

    QString     path_;      // <projectRoot>/inspector-layout.json
    QJsonObject document_;  // ⚠️ THE WHOLE DOCUMENT — see "patch, never reconstruct"
    bool        loaded_ = false;
};
