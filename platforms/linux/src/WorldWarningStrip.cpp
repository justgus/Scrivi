#include "WorldWarningStrip.hpp"
#include "GlyphIcon.hpp"

#include "ThemeColours.hpp"
#include "WorldStatusText.hpp"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

WorldWarningStrip::WorldWarningStrip(QWidget* parent, std::function<void()> onManageWorlds) : QFrame(parent)
{
    setObjectName(QStringLiteral("worldWarningStrip"));
    setFrameShape(QFrame::NoFrame);
    // Apple: an orange tint over a material, so it reads over the manuscript's text. Here: the window colour, a tenth
    // amber, nearly opaque.
    QPalette pal = palette();
    const QColor base = pal.color(QPalette::Window);
    const QColor amber(255, 149, 0);
    QColor tint = QColor::fromRgbF(base.redF() * 0.9 + amber.redF() * 0.1, base.greenF() * 0.9 + amber.greenF() * 0.1,
                                   base.blueF() * 0.9 + amber.blueF() * 0.1);
    tint.setAlphaF(0.94);
    pal.setColor(QPalette::Window, tint);
    setPalette(pal);
    setAutoFillBackground(true);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(10, 6, 10, 6);
    column->setSpacing(2);
    auto* top = new QHBoxLayout;
    top->setSpacing(6);
    top->addWidget(GlyphIcon::mark(QStringLiteral("inkwyn.warnings"), QSize(16, 16), this, amber));   // EP-060 SP-243 Q6: was "▲"
    headline_ = new QLabel(this);
    QFont bold = headline_->font();
    bold.setBold(true);
    headline_->setFont(bold);
    top->addWidget(headline_, 1);
    auto* manage = new QPushButton(tr("Manage Worlds…"), this);
    manage->setObjectName(QStringLiteral("worldWarningManage"));
    manage->setFlat(true);
    manage->setFocusPolicy(Qt::NoFocus);   // never takes focus from the manuscript
    connect(manage, &QPushButton::clicked, this, [onManageWorlds]() { if (onManageWorlds) { onManageWorlds(); } });
    top->addWidget(manage);
    auto* close = new QToolButton(this);
    close->setObjectName(QStringLiteral("worldWarningDismiss"));
    close->setIcon(GlyphIcon::icon(QStringLiteral("inkwyn.close"), GlyphIcon::Drawing::Small));
    close->setAutoRaise(true);
    close->setFocusPolicy(Qt::NoFocus);
    close->setToolTip(tr("Hide this notice. It returns if another world becomes unavailable."));
    close->setAccessibleName(tr("Hide world warning"));
    connect(close, &QToolButton::clicked, this, &WorldWarningStrip::dismiss);
    top->addWidget(close);
    column->addLayout(top);
    lines_ = new QLabel(this);
    lines_->setWordWrap(true);
    ThemeColours::applyTextColour(lines_, ThemeColours::deemphasised(lines_->palette()));
    column->addWidget(lines_);

    parent->installEventFilter(this);
    hide();
}

void WorldWarningStrip::setRows(const QList<Row>& rows, const QString& loadError)
{
    rows_ = rows;
    loadError_ = loadError;
    QSet<QString> ids;
    for (const Row& r : rows_) { ids.insert(r.worldID); }
    // Auto-reveal when a world not flagged before goes away.
    if (!(ids - lastSeenWorldIDs_).isEmpty()) { dismissed_ = false; }
    lastSeenWorldIDs_ = ids;
    refresh();
}

void WorldWarningStrip::setShownByView(bool shown)
{
    shownByView_ = shown;
    refresh();
}

void WorldWarningStrip::dismiss()
{
    dismissed_ = true;
    refresh();
}

QString WorldWarningStrip::text() const
{
    return headline_->text() + QLatin1Char('\n') + lines_->text();
}

void WorldWarningStrip::refresh()
{
    headline_->setText(rows_.size() == 1 ? tr("1 world is unavailable")
                                         : tr("%1 worlds are unavailable").arg(rows_.size()));
    QStringList lines;
    if (!loadError_.isEmpty()) { lines << loadError_; }
    for (const Row& r : rows_) {
        // ⚠️ "held pending", never anything that implies loss: the links ARE intact (Apple, I-0115).
        const QString links = r.pendingCount == 0   ? tr("No links are affected yet")
                              : r.pendingCount == 1 ? tr("1 link is held pending")
                                                    : tr("%1 links are held pending").arg(r.pendingCount);
        lines << tr("“%1” is %2. %3.").arg(r.displayName, WorldStatusText::writerDescription(r.status), links);
    }
    lines_->setText(lines.join(QLatin1Char('\n')));
    lines_->setVisible(!lines.isEmpty());
    setVisible(shownByView_ && !dismissed_ && (!rows_.isEmpty() || !loadError_.isEmpty()));
    if (isVisible()) {
        place();
        raise();
    }
}

void WorldWarningStrip::place()
{
    const QWidget* host = parentWidget();
    const int h = heightForWidth(host->width()) > 0 ? heightForWidth(host->width()) : sizeHint().height();
    setGeometry(0, host->height() - h, host->width(), h);
}

bool WorldWarningStrip::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize && isVisible()) { place(); }
    return QFrame::eventFilter(watched, event);
}
