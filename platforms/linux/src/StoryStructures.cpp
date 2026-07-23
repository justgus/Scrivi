#include "StoryStructures.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace scrivi::linux_app::story {

namespace {

// The built-in table, ported verbatim from Apple BuiltInStructure.bandTable. Equal
// initial proportions per structure (they sum to 1.0). "Custom" is not a preset — a
// custom layout is just an edited built-in (border-drag), stored as-is by the backend.
QList<Structure> makeBuiltIns()
{
    const double third   = 1.0 / 3.0;
    const double twelfth = 1.0 / 12.0;
    const double fifteenth = 1.0 / 15.0;

    return {
        {QStringLiteral("three-act"), QStringLiteral("Three Act Structure"), {
            {QStringLiteral("act-i"),   QStringLiteral("Act I"),   QStringLiteral("#5B8DD9"), third},
            {QStringLiteral("act-ii"),  QStringLiteral("Act II"),  QStringLiteral("#D9A05B"), third},
            {QStringLiteral("act-iii"), QStringLiteral("Act III"), QStringLiteral("#5BD98D"), third},
        }},
        {QStringLiteral("five-act"), QStringLiteral("Five Act Structure"), {
            {QStringLiteral("exposition"),     QStringLiteral("Exposition"),     QStringLiteral("#5B8DD9"), 0.2},
            {QStringLiteral("rising-action"),  QStringLiteral("Rising Action"),  QStringLiteral("#8D5BD9"), 0.2},
            {QStringLiteral("climax"),         QStringLiteral("Climax"),         QStringLiteral("#D95B5B"), 0.2},
            {QStringLiteral("falling-action"), QStringLiteral("Falling Action"), QStringLiteral("#D9A05B"), 0.2},
            {QStringLiteral("denouement"),     QStringLiteral("Denouement"),     QStringLiteral("#5BD98D"), 0.2},
        }},
        {QStringLiteral("heroes-journey"), QStringLiteral("Hero's Journey"), {
            {QStringLiteral("ordinary-world"),       QStringLiteral("Ordinary World"),        QStringLiteral("#5B8DD9"), twelfth},
            {QStringLiteral("call-to-adventure"),    QStringLiteral("Call to Adventure"),     QStringLiteral("#7B8DD9"), twelfth},
            {QStringLiteral("refusal"),              QStringLiteral("Refusal of the Call"),   QStringLiteral("#9B8DD9"), twelfth},
            {QStringLiteral("meeting-mentor"),       QStringLiteral("Meeting the Mentor"),    QStringLiteral("#BD8DD9"), twelfth},
            {QStringLiteral("crossing-threshold"),   QStringLiteral("Crossing the Threshold"),QStringLiteral("#D98DBD"), twelfth},
            {QStringLiteral("tests-allies-enemies"), QStringLiteral("Tests, Allies, Enemies"),QStringLiteral("#D98D9B"), twelfth},
            {QStringLiteral("approach"),             QStringLiteral("Approach"),              QStringLiteral("#D9A05B"), twelfth},
            {QStringLiteral("ordeal"),               QStringLiteral("Ordeal"),                QStringLiteral("#D9735B"), twelfth},
            {QStringLiteral("reward"),               QStringLiteral("Reward"),                QStringLiteral("#D95B5B"), twelfth},
            {QStringLiteral("road-back"),            QStringLiteral("The Road Back"),         QStringLiteral("#A05BD9"), twelfth},
            {QStringLiteral("resurrection"),         QStringLiteral("Resurrection"),          QStringLiteral("#7B5BD9"), twelfth},
            {QStringLiteral("return"),               QStringLiteral("Return with the Elixir"),QStringLiteral("#5B8DD9"), twelfth},
        }},
        {QStringLiteral("save-the-cat"), QStringLiteral("Save the Cat"), {
            {QStringLiteral("opening-image"),     QStringLiteral("Opening Image"),          QStringLiteral("#5B8DD9"), fifteenth},
            {QStringLiteral("theme-stated"),      QStringLiteral("Theme Stated"),           QStringLiteral("#6B8DD9"), fifteenth},
            {QStringLiteral("set-up"),            QStringLiteral("Set-Up"),                 QStringLiteral("#7B8DD9"), fifteenth},
            {QStringLiteral("catalyst"),          QStringLiteral("Catalyst"),               QStringLiteral("#8B8DD9"), fifteenth},
            {QStringLiteral("debate"),            QStringLiteral("Debate"),                 QStringLiteral("#9B8DD9"), fifteenth},
            {QStringLiteral("break-into-two"),    QStringLiteral("Break Into Two"),         QStringLiteral("#AB8DD9"), fifteenth},
            {QStringLiteral("b-story"),           QStringLiteral("B Story"),                QStringLiteral("#BD8DD9"), fifteenth},
            {QStringLiteral("fun-and-games"),     QStringLiteral("Fun and Games"),          QStringLiteral("#D98DBD"), fifteenth},
            {QStringLiteral("midpoint"),          QStringLiteral("Midpoint"),               QStringLiteral("#D98D9B"), fifteenth},
            {QStringLiteral("bad-guys-close-in"), QStringLiteral("Bad Guys Close In"),      QStringLiteral("#D9735B"), fifteenth},
            {QStringLiteral("all-is-lost"),       QStringLiteral("All Is Lost"),            QStringLiteral("#D95B5B"), fifteenth},
            {QStringLiteral("dark-night"),        QStringLiteral("Dark Night of the Soul"), QStringLiteral("#C05B5B"), fifteenth},
            {QStringLiteral("break-into-three"),  QStringLiteral("Break Into Three"),       QStringLiteral("#A05BD9"), fifteenth},
            {QStringLiteral("finale"),            QStringLiteral("Finale"),                 QStringLiteral("#7B5BD9"), fifteenth},
            {QStringLiteral("final-image"),       QStringLiteral("Final Image"),            QStringLiteral("#5B8DD9"), fifteenth},
        }},
        {QStringLiteral("freytag"), QStringLiteral("Freytag's Pyramid"), {
            {QStringLiteral("introduction"),   QStringLiteral("Introduction"),   QStringLiteral("#5B8DD9"), 0.2},
            {QStringLiteral("rising-action"),  QStringLiteral("Rising Action"),  QStringLiteral("#8D5BD9"), 0.2},
            {QStringLiteral("climax"),         QStringLiteral("Climax"),         QStringLiteral("#D95B5B"), 0.2},
            {QStringLiteral("falling-action"), QStringLiteral("Falling Action"), QStringLiteral("#D9A05B"), 0.2},
            {QStringLiteral("catastrophe"),    QStringLiteral("Catastrophe"),    QStringLiteral("#5BD98D"), 0.2},
        }},
        {QStringLiteral("kishotenketsu"), QStringLiteral("Kishōtenketsu"), {
            {QStringLiteral("ki"),    QStringLiteral("Ki — Introduction"), QStringLiteral("#5B8DD9"), 0.25},
            {QStringLiteral("sho"),   QStringLiteral("Shō — Development"), QStringLiteral("#8D5BD9"), 0.25},
            {QStringLiteral("ten"),   QStringLiteral("Ten — Twist"),       QStringLiteral("#D95B5B"), 0.25},
            {QStringLiteral("ketsu"), QStringLiteral("Ketsu — Conclusion"),QStringLiteral("#5BD98D"), 0.25},
        }},
        {QStringLiteral("in-medias-res"), QStringLiteral("In Medias Res"), {
            {QStringLiteral("action-open"),   QStringLiteral("Action Open"),   QStringLiteral("#D95B5B"), 0.25},
            {QStringLiteral("backstory"),     QStringLiteral("Backstory"),     QStringLiteral("#5B8DD9"), 0.25},
            {QStringLiteral("rising-action"), QStringLiteral("Rising Action"), QStringLiteral("#8D5BD9"), 0.25},
            {QStringLiteral("resolution"),    QStringLiteral("Resolution"),    QStringLiteral("#5BD98D"), 0.25},
        }},
    };
}

} // namespace

const QList<Structure>& builtInStructures()
{
    static const QList<Structure> kStructures = makeBuiltIns();
    return kStructures;
}

const Structure* structureForID(const QString& structureID)
{
    for (const Structure& s : builtInStructures()) {
        if (s.structureID == structureID) {
            return &s;
        }
    }
    return nullptr;
}

QString bandLayoutJSON(const QList<Band>& bands)
{
    // The ScriviCore contract (ScriviCore.cpp setStoryStructure/getStoryStructure) is an
    // OBJECT wrapping a "bands" array — {"bands":[{bandID,label,color,proportion},…]} —
    // NOT a bare array. Match it on both write and read.
    QJsonArray arr;
    for (const Band& b : bands) {
        QJsonObject o;
        o.insert(QStringLiteral("bandID"), b.bandID);
        o.insert(QStringLiteral("label"), b.label);
        o.insert(QStringLiteral("color"), b.color);
        o.insert(QStringLiteral("proportion"), b.proportion);
        arr.append(o);
    }
    QJsonObject root;
    root.insert(QStringLiteral("bands"), arr);
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

QList<Band> parseBandLayout(const QString& json)
{
    QList<Band> out;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) {
        return out;
    }
    for (const QJsonValue& v : doc.object().value(QStringLiteral("bands")).toArray()) {
        const QJsonObject o = v.toObject();
        out.append({o.value(QStringLiteral("bandID")).toString(),
                    o.value(QStringLiteral("label")).toString(),
                    o.value(QStringLiteral("color")).toString(),
                    o.value(QStringLiteral("proportion")).toDouble()});
    }
    return out;
}

} // namespace scrivi::linux_app::story
