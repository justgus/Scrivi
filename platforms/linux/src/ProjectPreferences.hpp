#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "ManuscriptTypography.hpp"

class ScriviBridge;

// EP-048 S5a (SP-182, T-0609) — per-project preferences that TRAVEL with the project, on Linux.
//
// ✅ Apple's shape (`Scrivi/App/ProjectPreferences.swift`, EP-047 AC1–AC3): the settings live in the package's
// `project-settings.json` (through ScriviCore, `scrivi_get/put_project_settings`); the TITLE is `project.json`'s own
// (`scrivi_set_project_title`).
// ⚠️ The document is shared with the Mac and with later builds, so it is kept WHOLE: a save merges this class's keys into
// the document it read and writes all of it back ([I-0215]: never drop a key you did not write).
// ⚠️ A key is written only once the writer has CHOSEN it, so a save of another setting never pins today's default.
// (⚠️ Apple always writes `subtitle` and `showChapterTitles` on a save; Linux does not write `showChapterTitles` at all
// until [SP-184] builds it, and writes `subtitle` only once edited — the same file either way.)
// ⛔ An UNREADABLE file is never overwritten on open; only an explicit change by the writer replaces it.
//
// One per open project, owned by its `EditorShell`; built from the settings the load's worker read.
class ProjectPreferences : public QObject
{
    Q_OBJECT

public:
    // `fetch` is `getProjectSettings`'s result ({status, document?, message?}); `title` is `project.json`'s.
    ProjectPreferences(ScriviBridge* bridge, QString projectRootPath, const QString& title, const QVariantMap& fetch,
                       QObject* parent = nullptr);

    [[nodiscard]] QString title() const { return title_; }
    void setTitle(const QString& title);
    [[nodiscard]] QString subtitle() const { return subtitle_; }
    void setSubtitle(const QString& subtitle);

    // ✅ EP-047 P2, P5a, P6: a BUNDLED face's name and a size, per project. Absent = Literata, 16 pt.
    [[nodiscard]] QString typeface() const { return typeface_; }
    void setTypeface(const QString& name);
    [[nodiscard]] double textSize() const { return textSize_; }
    void setTextSize(double size);
    [[nodiscard]] ManuscriptTypography typography() const { return {typeface_, textSize_}; }

    // ✅ EP-047 P11: Markup Hints — absent = ON. View ▸ Show Markup Hints (Ctrl+Shift+H).
    [[nodiscard]] bool markupHints() const { return markupHints_; }
    void setMarkupHints(bool on);

    // Why the settings file could not be read (empty when it could).
    [[nodiscard]] QString unreadableMessage() const { return unreadable_; }

    static constexpr const char* kSubtitleKey = "subtitle";
    static constexpr const char* kTypefaceKey = "typeface";
    static constexpr const char* kTextSizeKey = "textSize";
    static constexpr const char* kMarkupHintsKey = "markupHints";

signals:
    void typographyChanged();
    void markupHintsChanged(bool on);
    void titleChanged(const QString& title);

private:
    void save();

    ScriviBridge* bridge_ = nullptr;
    QString root_;
    QJsonObject document_;   // the whole file as read — keys this build does not know included
    QString unreadable_;
    QString title_;
    QString subtitle_;
    QString typeface_;
    double textSize_ = ManuscriptTypography::kDefaultSize;
    bool markupHints_ = true;
    bool subtitleChosen_ = false;
    bool typefaceChosen_ = false;
    bool textSizeChosen_ = false;
    bool markupHintsChosen_ = false;
};
