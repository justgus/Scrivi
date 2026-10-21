#pragma once

// RangeDelta — EP-053 S2a (SP-187 / T-0615): the change a range operation made, decoded from `scrivi_range_paste` /
// `scrivi_range_cut`'s envelope (design v0.2 P3) — field for field the same as Apple's `RangeDelta` (`ScriviEngine.swift`).
// It is what the editor applies to its scene list instead of re-opening the project (S2b, [SP-191]).
// ⚠️ The C ABI OMITS an empty list, so an absent key is an empty list.

#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

struct RangeDelta
{
    enum class Change { Created, Rewritten, Moved };

    struct Scene {
        Change  change = Change::Rewritten;
        QString sceneID;
        QString chapterID;
        QString title;
        QString chapterTitle;
        QString metadataPath;
        QString contentPath;
        QString chapterMetadataPath;
        QString afterSceneID;   // the scene before it in reading order now; empty = the manuscript's first
        QString text;           // the body as written; empty for Moved (its text is unchanged)
        bool    hasText = false;
    };

    struct Chapter {
        QString chapterID;
        QString title;
        QString metadataPath;
        QString afterChapterID;
    };

    QList<Scene>   scenes;
    QStringList    removedSceneIDs;
    QList<Chapter> createdChapters;
    QStringList    removedChapterIDs;

    // `delta` is the envelope result's "delta" map (as `ScriviBridge::parseEnvelope` returns it).
    static RangeDelta fromVariant(const QVariantMap& delta)
    {
        RangeDelta out;
        for (const QVariant& v : delta.value(QStringLiteral("scenes")).toList()) {
            const QVariantMap m = v.toMap();
            Scene s;
            const QString change = m.value(QStringLiteral("change")).toString();
            s.change = change == QLatin1String("created") ? Change::Created
                     : change == QLatin1String("moved")   ? Change::Moved
                                                          : Change::Rewritten;
            s.sceneID             = m.value(QStringLiteral("sceneID")).toString();
            s.chapterID           = m.value(QStringLiteral("chapterID")).toString();
            s.title               = m.value(QStringLiteral("title")).toString();
            s.chapterTitle        = m.value(QStringLiteral("chapterTitle")).toString();
            s.metadataPath        = m.value(QStringLiteral("metadataPath")).toString();
            s.contentPath         = m.value(QStringLiteral("contentPath")).toString();
            s.chapterMetadataPath = m.value(QStringLiteral("chapterMetadataPath")).toString();
            s.afterSceneID        = m.value(QStringLiteral("afterSceneID")).toString();
            s.hasText             = m.contains(QStringLiteral("text"));
            s.text                = m.value(QStringLiteral("text")).toString();
            out.scenes.append(s);
        }
        for (const QVariant& v : delta.value(QStringLiteral("removedSceneIDs")).toList()) {
            out.removedSceneIDs.append(v.toString());
        }
        for (const QVariant& v : delta.value(QStringLiteral("createdChapters")).toList()) {
            const QVariantMap m = v.toMap();
            out.createdChapters.append({m.value(QStringLiteral("chapterID")).toString(),
                                        m.value(QStringLiteral("title")).toString(),
                                        m.value(QStringLiteral("metadataPath")).toString(),
                                        m.value(QStringLiteral("afterChapterID")).toString()});
        }
        for (const QVariant& v : delta.value(QStringLiteral("removedChapterIDs")).toList()) {
            out.removedChapterIDs.append(v.toString());
        }
        return out;
    }
};
