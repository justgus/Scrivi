#pragma once

#include <QCoreApplication>
#include <QString>

// WorldStatusText — writer-facing phrasing for a core `WorldStatus` name
// (EP-035 / SP-127, T-0492).
//
// ## Why this exists as a shared header
//
// The phrasing was written for SP-125's Scene Inspector status line and lived as
// a static member of `SceneInspector`. SP-127 adds a second reader — the Worlds
// dialog — and a second surface needing the same words is exactly the moment the
// wording either gets HOISTED or gets RESTATED.
//
// ⚠️ A restatement would be a defect on sight. CLAUDE.md's standing rule is
// written about `ObjectKind` lists, but its reasoning is about ANY table that
// partitions a core enum: *"a list rots without being edited"*. If the core ever
// adds a status, or a platform refinement starts emitting `unmounted` where it
// previously said `unavailable`, a second copy goes stale silently while its own
// text still reads correctly.
//
// ⚠️ It is deliberately NOT a member of either surface. A Worlds dialog reaching
// into `SceneInspector` for a string would couple two unrelated views through the
// one that happened to be built first.
//
// ## The governing rule (Doc 2 §7.2.1, and I-0115, which was this shipped)
//
// ⚠️ `missing` is reported ONLY when the core positively established it. Anything
// undetermined arrives as `unavailable` and must be REPEATED as such, never
// upgraded — a wrong "missing" invites destructive remedies (restore from backup,
// clear references) against a world that is perfectly intact on a volume we
// merely cannot see.
namespace WorldStatusText {

// ⚠️ QT_TRANSLATE_NOOP-style translation via QCoreApplication::translate rather
// than Q_DECLARE_TR_FUNCTIONS: that macro expands to CLASS MEMBERS (it emits
// `public:`/`private:`) and does not compile inside a namespace.
inline QString tr(const char* text)
{
    return QCoreApplication::translate("WorldStatusText", text);
}

// Mirrors Apple's `WorldStatus.writerDescription` (ScriviError.swift:115) word
// for word, so the two platforms describe the same condition identically.
//
// ⚠️ The fallback is `unavailable`, NOT the raw status name: an unrecognised
// status is by definition one this build cannot explain, and the honest generic
// beats leaking an enum name at a writer.
inline QString writerDescription(const QString& statusName)
{
    if (statusName == QLatin1String("available"))   { return tr("available"); }
    if (statusName == QLatin1String("offline"))     { return tr("offline"); }
    if (statusName == QLatin1String("unmounted"))   { return tr("on a disconnected volume"); }
    if (statusName == QLatin1String("missing"))     { return tr("missing"); }
    return tr("unavailable");
}

// Whether a status name means the world resolved.
//
// ⚠️ Phrased as "is this exactly `available`" rather than "is this one of the bad
// ones", so a status added to the core later reads as NOT-available by default.
// ⚠️ The opposite spelling would silently treat an unknown new status as healthy.
inline bool isAvailable(const QString& statusName)
{
    return statusName == QLatin1String("available");
}

}  // namespace WorldStatusText
