#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

class ScriviBridge;

// ObjectKindScope — the object-kind scope partition, DERIVED from ScriviCore
// rather than restated in Qt (EP-035 / SP-125, T-0480). The Linux mirror of
// Apple's Scrivi/Engine/ObjectKindScope.swift (SP-116, T-0429, design doc D5).
//
// ## Why this type exists
//
// CLAUDE.md's standing rule: *any list that partitions or enumerates
// `ObjectKind` must derive from `kAllStorableKinds` and
// `objectKindIsWorldScoped()` — in C++, Swift, and QML alike. A restatement is a
// defect on sight, even when it is currently correct.*
//
// ⚠️ That rule has been broken EIGHT times, once badly enough to block object
// creation in the app entirely (SP-104). **Occurrence five was in Swift** — a
// new platform layer is exactly where this class of defect recurs, and Qt is no
// more immune than SwiftUI was. A hardcoded kind list anywhere in
// platforms/linux/ would be occurrence NINE.
//
// ⚠️ The original cause was STRUCTURAL, not careless: nothing in the ABI exposed
// a kind's scope, so a platform layer could not derive what the boundary never
// told it, and restated it instead. `scrivi_list_object_kinds` (SP-116) now
// does; this type is the Qt side of that cure.
//
// ## Two properties worth keeping — both carried over from the Swift original
//
//  • **Loaded once, from the core.** A kind's scope is a property of the model,
//    not of a project, so the table needs no project root and never varies by
//    document. Fetched on first use and cached for the process.
//
//  • ⚠️ **The fallback is not a second copy of the rule.** If the core cannot be
//    reached, `isWorldScoped` returns TRUE for any kind it does not recognise.
//    That is deliberate: treating a kind as world-scoped makes the app *ask* for
//    a world, which fails loudly and recoverably. Guessing project-scope would
//    silently pass an empty `worldID` and reproduce the SP-104 outage.
//    **It is a safety default, not a restatement — it names no kinds.**
namespace ObjectKindScope {

// One kind exactly as ScriviCore reports it. `subdir` travels because the core
// owns the on-disk layout; nothing here derives a path from a kind name.
struct KindInfo {
    QString kind;
    QString subdir;
    bool    isWorldScoped = false;
};

// Every storable kind the core knows, in the core's own order, loading on first
// use through `bridge`. Empty only when the core could not be reached.
//
// ⚠️ `world` is deliberately absent from the core's list: it is a container
// created by scrivi_create_world, not a storable object kind.
const QList<KindInfo>& kinds(ScriviBridge* bridge);

// Whether objects of `kind` live in a world package rather than the project.
// Unknown kinds report TRUE — see the fallback rationale above.
bool isWorldScoped(ScriviBridge* bridge, const QString& kind);

// True once the core has answered at least once. A surface uses this to tell
// "no kinds because the core is unreachable" apart from any other empty state
// (T-0483: absence is never silently collapsed).
bool isLoaded(ScriviBridge* bridge);

} // namespace ObjectKindScope
