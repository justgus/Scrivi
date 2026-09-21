#include "InspectorLayoutStore.hpp"

#include "ScriviBridge.hpp"

#include <QJsonArray>
#include <QJsonDocument>
// ⛔ NO <QFile>, <QSaveFile> or <QDir>: T-0537 moved every byte of this file's I/O
// into ScriviCore. Reintroducing one of these includes is the tell that the
// [I-0197] bypass is coming back.

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

void InspectorLayoutStore::load(ScriviBridge* bridge, const QString& projectRootPath)
{
    bridge_ = bridge;
    projectRootPath_ = projectRootPath;
    document_ = QJsonObject{};
    loaded_ = false;
    unreadable_ = false;

    if (!bridge_ || projectRootPath.isEmpty()) {
        return;
    }

    const QVariantMap r = bridge_->getInspectorLayout(projectRootPath);
    if (bridge_->lastCallFailed()) {
        // ⚠️ The CALL failed, not the document. Defaults in memory, and ⛔ `loaded_`
        // stays false so no setter can write over a layout we never saw.
        document_ = makeDefaultDocument();
        return;
    }

    const QString status = r.value(QStringLiteral("status")).toString();

    if (status == QLatin1String("absent")) {
        // ✅ NORMAL, NOT AN ERROR. A project created before this file existed has
        // no layout; so does one whose inspector has never been touched. Defaults
        // are held in memory and only reach disk if something is actually set.
        document_ = makeDefaultDocument();
        loaded_ = true;
        return;
    }

    if (status != QLatin1String("ok")) {
        // ⚠️ "unreadable" (or anything unrecognised). ⛔ `loaded_` STAYS FALSE, which
        // makes every setter a no-op — so a damaged layout is left exactly as found
        // for a human to look at. The core did not overwrite it either.
        // ✅ Losing a layout is annoying; destroying the evidence of how it broke is
        // worse.
        document_ = makeDefaultDocument();
        unreadable_ = (status == QLatin1String("unreadable"));
        return;
    }

    // ⚠️ THE WHOLE DOCUMENT is kept, including every key this build does not
    // understand (`stackSort`, `defaultStacks`, `scenes`, and anything a future
    // Scrivi adds). This is what makes the round trip lossless.
    // ✅ The core hands it back verbatim — it never interprets it.
    const QVariantMap doc = r.value(QStringLiteral("document")).toMap();
    document_ = QJsonObject::fromVariantMap(doc);
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
    if (!bridge_ || projectRootPath_.isEmpty() || !loaded_) {
        return false;
    }
    // ✅ ATOMICITY IS THE CORE'S JOB (EP-041): it writes a temp and renames, the
    // same discipline `QSaveFile` gave us here before T-0537.
    // ⛔ Do not reintroduce a write — that is the [I-0197] bypass.
    const QString json =
        QString::fromUtf8(QJsonDocument(document_).toJson(QJsonDocument::Compact));
    bridge_->putInspectorLayout(projectRootPath_, json);
    return !bridge_->lastCallFailed();
}
