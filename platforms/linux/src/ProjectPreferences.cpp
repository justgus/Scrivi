#include "ProjectPreferences.hpp"

#include <QJsonDocument>

#include "ScriviBridge.hpp"

#include <algorithm>

ProjectPreferences::ProjectPreferences(ScriviBridge* bridge, QString projectRootPath, const QString& title,
                                       const QVariantMap& fetch, QObject* parent)
    : QObject(parent), bridge_(bridge), root_(std::move(projectRootPath)), title_(title)
{
    const QString status = fetch.value(QStringLiteral("status")).toString();
    if (status == QStringLiteral("ok")) {
        document_ = QJsonObject::fromVariantMap(fetch.value(QStringLiteral("document")).toMap());
    } else if (status == QStringLiteral("unreadable")) {
        const QString m = fetch.value(QStringLiteral("message")).toString();
        unreadable_ = m.isEmpty() ? QStringLiteral("unreadable") : m;
    }
    const auto has = [this](const char* key) { return document_.contains(QLatin1String(key)); };
    subtitle_ = document_.value(QLatin1String(kSubtitleKey)).toString();
    typeface_ = has(kTypefaceKey) ? document_.value(QLatin1String(kTypefaceKey)).toString()
                                  : BundledFonts::manifest().defaultFace;
    textSize_ = has(kTextSizeKey) ? document_.value(QLatin1String(kTextSizeKey)).toDouble(ManuscriptTypography::kDefaultSize)
                                  : ManuscriptTypography::kDefaultSize;
    markupHints_ = has(kMarkupHintsKey) ? document_.value(QLatin1String(kMarkupHintsKey)).toBool(true) : true;
    subtitleChosen_ = has(kSubtitleKey);
    typefaceChosen_ = has(kTypefaceKey);
    textSizeChosen_ = has(kTextSizeKey);
    markupHintsChosen_ = has(kMarkupHintsKey);
}

void ProjectPreferences::setTitle(const QString& title)
{
    if (title == title_) {
        return;
    }
    title_ = title;
    // ⛔ An empty title is not written (the core refuses it; the window shows "Project" meanwhile).
    const QString t = title.trimmed();
    if (!t.isEmpty() && bridge_ != nullptr) {
        bridge_->setProjectTitle(root_, t);
    }
    emit titleChanged(title_);
}

void ProjectPreferences::setSubtitle(const QString& subtitle)
{
    if (subtitle == subtitle_ && subtitleChosen_) {
        return;
    }
    subtitle_ = subtitle;
    subtitleChosen_ = true;
    save();
}

void ProjectPreferences::setTypeface(const QString& name)
{
    if (name == typeface_ && typefaceChosen_) {
        return;
    }
    typeface_ = name;
    typefaceChosen_ = true;
    save();
    emit typographyChanged();
}

void ProjectPreferences::setTextSize(double size)
{
    size = std::clamp(size, ManuscriptTypography::kMinSize, ManuscriptTypography::kMaxSize);
    if (size == textSize_ && textSizeChosen_) {
        return;
    }
    textSize_ = size;
    textSizeChosen_ = true;
    save();
    emit typographyChanged();
}

void ProjectPreferences::setMarkupHints(bool on)
{
    if (on == markupHints_ && markupHintsChosen_) {
        return;
    }
    markupHints_ = on;
    markupHintsChosen_ = true;
    save();
    emit markupHintsChanged(on);
}

void ProjectPreferences::save()
{
    if (subtitleChosen_) { document_.insert(QLatin1String(kSubtitleKey), subtitle_); }
    if (typefaceChosen_) { document_.insert(QLatin1String(kTypefaceKey), typeface_); }
    if (textSizeChosen_) { document_.insert(QLatin1String(kTextSizeKey), textSize_); }
    if (markupHintsChosen_) { document_.insert(QLatin1String(kMarkupHintsKey), markupHints_); }
    if (bridge_ == nullptr) {
        return;
    }
    const QVariantMap r = bridge_->putProjectSettings(
        root_, QString::fromUtf8(QJsonDocument(document_).toJson(QJsonDocument::Indented)));
    if (r.value(QStringLiteral("saved")).toBool()) {
        unreadable_.clear();
    }
}
