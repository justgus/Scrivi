#include "InspectorLayoutStore.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

namespace {

// The tabs this build understands. ⚠️ These are TAB identifiers, not ObjectKind
// names — the standing "derive, never restate" rule is about `ObjectKind` and
// does not apply here. There is no endpoint that enumerates inspector tabs; they
// are a UI concept Apple defines in `InspectorTab` (InspectorCard.swift:19).
constexpr const char* kTabWriting       = "writing";
constexpr const char* kTabWorldbuilding = "worldbuilding";
constexpr const char* kTabProperties    = "properties";

bool isKnownTab(const QString& tab)
{
    return tab == QLatin1String(kTabWriting)
        || tab == QLatin1String(kTabWorldbuilding)
        || tab == QLatin1String(kTabProperties);
}

// Apple's ruled defaults for a fresh project (InspectorLayoutStore.swift:75):
// Worldbuilding ships EMPTY; Writing ships with empty tags/outline/todo cards.
//
// ⚠️ Written ONLY when no file exists. An existing file is never "upgraded" to
// this shape — that would be reconstruction by another name.
QJsonObject makeDefaultDocument()
{
    QJsonObject writingStack;
    QJsonArray writingCards;
    for (const char* type : {"tags", "outline", "todo"}) {
        QJsonObject card;
        card.insert(QStringLiteral("type"), QLatin1String(type));
        card.insert(QStringLiteral("collapsed"), false);
        writingCards.append(card);
    }

    QJsonObject defaultStacks;
    defaultStacks.insert(QLatin1String(kTabWriting), writingCards);
    defaultStacks.insert(QLatin1String(kTabWorldbuilding), QJsonArray{});

    QJsonObject stackSort;
    stackSort.insert(QLatin1String(kTabWriting), QStringLiteral("manual"));
    stackSort.insert(QLatin1String(kTabWorldbuilding), QStringLiteral("manual"));

    QJsonObject doc;
    doc.insert(QStringLiteral("schema"), QLatin1String(InspectorLayoutStore::kSchemaID));
    doc.insert(QStringLiteral("selectedTab"), QLatin1String(kTabWriting));
    doc.insert(QStringLiteral("inspectorHidden"), false);
    doc.insert(QStringLiteral("defaultStacks"), defaultStacks);
    doc.insert(QStringLiteral("stackSort"), stackSort);
    doc.insert(QStringLiteral("scenes"), QJsonObject{});
    return doc;
}

} // namespace

void InspectorLayoutStore::load(const QString& projectRootPath)
{
    path_.clear();
    document_ = QJsonObject{};
    loaded_ = false;

    if (projectRootPath.isEmpty()) {
        return;
    }
    path_ = QDir(projectRootPath).filePath(QStringLiteral("inspector-layout.json"));

    QFile f(path_);
    if (!f.open(QIODevice::ReadOnly)) {
        // ⚠️ A MISSING file is not an error — a project created before this file
        // existed has none, and Apple's own loader falls back the same way.
        // Defaults are held in memory and only reach disk if something is set.
        document_ = makeDefaultDocument();
        loaded_ = true;
        return;
    }

    QJsonParseError err{};
    const QJsonDocument parsed = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();

    if (err.error != QJsonParseError::NoError || !parsed.isObject()) {
        // ⚠️ A CORRUPT file is deliberately NOT overwritten with defaults here.
        // `loaded_` stays false, which makes every setter a no-op, so a damaged
        // layout is left exactly as found for a human to look at rather than
        // being silently replaced. Losing a layout is annoying; destroying the
        // evidence of how it broke is worse.
        document_ = makeDefaultDocument();
        return;
    }

    // ⚠️ THE WHOLE DOCUMENT is kept, including every key this build does not
    // understand (`stackSort`, `defaultStacks`, `scenes`, and anything a future
    // Scrivi adds). This is what makes the round trip lossless.
    document_ = parsed.object();
    loaded_ = true;
}

QString InspectorLayoutStore::selectedTab() const
{
    const QString tab = document_.value(QStringLiteral("selectedTab")).toString();
    // ⚠️ Unknown or absent degrades to Writing — Apple's ruled default — rather
    // than failing. A newer Scrivi may name a tab this build does not have.
    return isKnownTab(tab) ? tab : QString::fromLatin1(kTabWriting);
}

void InspectorLayoutStore::setSelectedTab(const QString& tab)
{
    if (!loaded_ || !isKnownTab(tab)) {
        return;
    }
    if (document_.value(QStringLiteral("selectedTab")).toString() == tab) {
        return;   // no write for a no-op selection
    }
    // ⚠️ ONE key is touched. Everything else in `document_` is carried through
    // byte-for-byte by the serialiser.
    document_.insert(QStringLiteral("selectedTab"), tab);
    // Keep the schema stamp honest for a file we created from defaults.
    if (!document_.contains(QStringLiteral("schema"))) {
        document_.insert(QStringLiteral("schema"), QLatin1String(kSchemaID));
    }
    save();
}

bool InspectorLayoutStore::save() const
{
    if (path_.isEmpty() || !loaded_) {
        return false;
    }
    // QSaveFile writes to a temporary and renames on commit, so a crash or a
    // full disk cannot leave a truncated layout file behind.
    QSaveFile f(path_);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    // Indented to match Apple's output, so a project that moves between
    // platforms does not churn the file's formatting in version control.
    f.write(QJsonDocument(document_).toJson(QJsonDocument::Indented));
    return f.commit();
}
