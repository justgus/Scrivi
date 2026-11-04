#pragma once

// RangeVerify — EP-053 S4 ([SP-189], T-0617): the platform half of the BACKGROUND SYNC VERIFICATION (design P4, R3).
//
// After a structural delta is applied, a worker reads what the DISK holds for the files it named
// (`scrivi_range_verify_read`) and this compares it with the scene list, by the rule the core documents there (Q1):
// each touched chapter's scenes in order, each written scene's text, and the chapter order when chapters came or went.
// ✅ The disk wins, silently (design Q3): a text, or an order within a chapter, is corrected IN PLACE — as a delta, applied
// the way every structural delta is. ✅ Q2: a scene the writer typed into since is skipped (its newer text saves as usual).
// ⚠️ What a delta cannot fix — a scene on disk the list does not know, a chapter gone from under its scenes, another chapter
// order — is logged, never guessed at. Apple: `RangeVerify` in `ViewportSceneLoader.swift`, the same rule.

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "RangeDelta.hpp"
#include "SceneDocument.hpp"

namespace RangeVerify {

struct Request {
    QStringList                     chapters;       // each chapter sidecar the delta touched, removed or created
    QList<QPair<QString, QString>>  scenes;         // (sceneID, contentPath): each scene whose text the delta wrote
    bool                            chapterOrder = false;
    QHash<QString, QString>         expected;       // sceneID → the text the list held once the delta was applied (Q2)

    bool isEmpty() const { return chapters.isEmpty() && scenes.isEmpty(); }
    QString toJson() const
    {
        QJsonObject o;
        o.insert(QStringLiteral("chapters"), QJsonArray::fromStringList(chapters));
        QJsonArray s;
        for (const auto& [id, path] : scenes) {
            s.append(QJsonObject{{QStringLiteral("sceneID"), id}, {QStringLiteral("contentPath"), path}});
        }
        o.insert(QStringLiteral("scenes"), s);
        o.insert(QStringLiteral("chapterOrder"), chapterOrder);
        return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
    }
};

// BEFORE the delta is applied: `before` still knows the chapters the removed scenes were in.
inline Request requestFor(const RangeDelta& delta, const QList<SceneSegment>& before)
{
    Request r;
    const auto chapter = [&r](const QString& path) {
        if (!path.isEmpty() && !r.chapters.contains(path)) { r.chapters.append(path); }
    };
    for (const RangeDelta::Scene& s : delta.scenes) {
        chapter(s.chapterMetadataPath);
        if (s.hasText) { r.scenes.append({s.sceneID, s.contentPath}); }
    }
    const QSet<QString> removed(delta.removedSceneIDs.cbegin(), delta.removedSceneIDs.cend());
    const QSet<QString> removedChapters(delta.removedChapterIDs.cbegin(), delta.removedChapterIDs.cend());
    for (const SceneSegment& seg : before) {
        if (removed.contains(seg.sceneID) || removedChapters.contains(seg.chapterID)) { chapter(seg.chapterMetadataPath); }
    }
    for (const RangeDelta::Chapter& c : delta.createdChapters) { chapter(c.metadataPath); }
    r.chapterOrder = !delta.createdChapters.isEmpty() || !delta.removedChapterIDs.isEmpty();
    return r;
}

// Once the delta is applied: the text each verified scene holds now — what "typed into since" is measured against.
inline void snapshot(Request& r, const QList<SceneSegment>& now)
{
    for (const auto& [id, path] : r.scenes) {
        for (const SceneSegment& seg : now) {
            if (seg.sceneID == id) { r.expected.insert(id, seg.text); }
        }
    }
}

struct Outcome {
    RangeDelta  correction;      // apply it: the disk wins
    int         corrected = 0;   // scenes it puts right
    QStringList skipped;         // typed into since (Q2)
    QStringList uncorrectable;   // what a delta cannot put right — logged
};

// `disk` is `scrivi_range_verify_read`'s result; `now` the scene list as it is when the read comes back.
inline Outcome compare(const Request& r, const QVariantMap& disk, const QList<SceneSegment>& now)
{
    Outcome out;
    QHash<QString, int> byID, byPath;
    for (int i = 0; i < now.size(); ++i) {
        byID.insert(now[i].sceneID, i);
        byPath.insert(now[i].metadataPath, i);
    }
    QHash<QString, RangeDelta::Scene> fix;   // sceneID → its correction
    QHash<QString, QStringList> reordered;   // chapter sidecar → its scenes' IDs in the disk's order
    const auto change = [&](int i) -> RangeDelta::Scene& {
        const SceneSegment& seg = now[i];
        auto it = fix.find(seg.sceneID);
        if (it == fix.end()) {
            RangeDelta::Scene s;
            s.change = RangeDelta::Change::Moved;
            s.sceneID = seg.sceneID;
            s.chapterID = seg.chapterID;
            s.title = seg.title;
            s.chapterTitle = seg.chapterTitle;
            s.metadataPath = seg.metadataPath;
            s.contentPath = seg.contentPath;
            s.chapterMetadataPath = seg.chapterMetadataPath;
            s.afterSceneID = i > 0 ? now[i - 1].sceneID : QString();
            it = fix.insert(seg.sceneID, s);
        }
        return *it;
    };

    // 2. Texts.
    for (const QVariant& v : disk.value(QStringLiteral("scenes")).toList()) {
        const QVariantMap s = v.toMap();
        const QString id = s.value(QStringLiteral("sceneID")).toString();
        const int i = byID.value(id, -1);
        if (i < 0 || !s.value(QStringLiteral("present")).toBool()) {
            out.uncorrectable.append(QStringLiteral("scene %1: %2").arg(id, i < 0 ? QStringLiteral("not in the list")
                                                                                  : QStringLiteral("not on disk")));
            continue;
        }
        const QString text = s.value(QStringLiteral("text")).toString();
        if (text == now[i].text) { continue; }
        if (now[i].text != r.expected.value(id)) {
            out.skipped.append(id);
            continue;
        }
        RangeDelta::Scene& c = change(i);
        c.change = RangeDelta::Change::Rewritten;
        c.text = text;
        c.hasText = true;
    }
    // 1. Each chapter's scenes, in order.
    for (const QVariant& v : disk.value(QStringLiteral("chapters")).toList()) {
        const QVariantMap c = v.toMap();
        const QString path = c.value(QStringLiteral("chapterMetadataPath")).toString();
        QStringList mine;
        int first = -1;
        for (int i = 0; i < now.size(); ++i) {
            if (now[i].chapterMetadataPath == path) {
                mine.append(now[i].metadataPath);
                first = first < 0 ? i : first;
            }
        }
        const QStringList theirs = c.value(QStringLiteral("scenes")).toStringList();
        if (!c.value(QStringLiteral("present")).toBool()) {
            if (!mine.isEmpty()) { out.uncorrectable.append(QStringLiteral("chapter %1: not on disk").arg(path)); }
            continue;
        }
        if (mine == theirs) { continue; }
        if (QSet<QString>(mine.cbegin(), mine.cend()) != QSet<QString>(theirs.cbegin(), theirs.cend())) {
            out.uncorrectable.append(QStringLiteral("chapter %1: other scenes on disk").arg(path));
            continue;
        }
        // The same scenes in another order: each after the one the disk puts before it.
        QString after = first > 0 ? now[first - 1].sceneID : QString();
        for (const QString& p : theirs) {
            RangeDelta::Scene& s = change(byPath.value(p));
            s.afterSceneID = after;
            after = s.sceneID;
            reordered[path].append(s.sceneID);
        }
    }
    // 3. The chapter order.
    if (disk.contains(QStringLiteral("chapterOrder"))) {
        QStringList mine;
        for (const SceneSegment& seg : now) {
            if (mine.isEmpty() || mine.last() != seg.chapterMetadataPath) { mine.append(seg.chapterMetadataPath); }
        }
        if (mine != disk.value(QStringLiteral("chapterOrder")).toStringList()) {
            out.uncorrectable.append(QStringLiteral("the chapter order"));
        }
    }
    // In reading order, as a delta is applied — a reordered chapter's in the DISK's order, so each entry's `afterSceneID`
    // is already in place when it is placed.
    for (const SceneSegment& seg : now) {
        if (const auto it = reordered.constFind(seg.chapterMetadataPath); it != reordered.constEnd()) {
            if (!it->isEmpty()) {
                for (const QString& id : *it) { out.correction.scenes.append(fix.value(id)); }
                reordered[seg.chapterMetadataPath].clear();   // emitted once
            }
        } else if (fix.contains(seg.sceneID)) {
            out.correction.scenes.append(fix.value(seg.sceneID));
        }
    }
    out.corrected = int(out.correction.scenes.size());
    return out;
}

}   // namespace RangeVerify
