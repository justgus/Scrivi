#include "GlyphIcon.hpp"

#include <QFile>
#include <QGuiApplication>
#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSvgRenderer>
#include <QToolButton>

namespace GlyphIcon {
namespace {

QString path(const QString& name, const char* size)
{
    return QStringLiteral(":/glyphs/%1-%2.svg").arg(name, QLatin1String(size));
}

class GlyphEngine : public QIconEngine {
public:
    GlyphEngine(QString name, Drawing drawing, QColor tint) : name_(std::move(name)), drawing_(drawing), tint_(tint) {}

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State) override
    {
        const qreal dpr = painter->device() != nullptr ? painter->device()->devicePixelRatioF() : 1.0;
        painter->drawPixmap(rect, render(rect.size(), dpr, mode));
    }
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State) override { return render(size, 1.0, mode); }
    QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State, qreal scale) override
    {
        // ⚠️ Qt changed what `size` is: before 6.8 QIcon::pixmap passes `size * devicePixelRatio` (qicon.cpp v6.4.2:837); since
        // 6.8 the device-independent size (v6.10.0:933). Read as one, it rendered a 24-pt icon at 2× as 96 px (glyph_smoke §4).
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
        const QSize logical = (QSizeF(size) / scale).toSize();
#else
        const QSize logical = size;
#endif
        return render(logical, scale, mode);
    }
    QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override { return size; }
    QIconEngine* clone() const override { return new GlyphEngine(name_, drawing_, tint_); }
    QString key() const override { return QStringLiteral("inkwyn-glyph"); }

private:
    QPixmap render(const QSize& size, qreal dpr, QIcon::Mode mode) const
    {
        if (size.isEmpty()) { return {}; }
        const char* which = "large";
        switch (drawing_) {
        case Drawing::Small:  which = "small"; break;
        case Drawing::Medium: which = "medium"; break;
        case Drawing::Large:  which = "large"; break;
        case Drawing::BySize: which = size.height() <= 16 ? "small" : size.height() <= 24 ? "medium" : "large"; break;
        }
        QSvgRenderer svg(path(name_, which));
        if (!svg.isValid()) { return {}; }

        const QSize px = size * dpr;
        QImage image(px, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        QSizeF fit = svg.viewBoxF().size();
        fit.scale(QSizeF(px), Qt::KeepAspectRatio);   // aspect fit, centred
        svg.render(&p, QRectF(QPointF((px.width() - fit.width()) / 2, (px.height() - fit.height()) / 2), fit));
        // Tint: keep the drawing's alpha, take the palette's colour (as a template image / SF Symbol is).
        const QPalette pal = QGuiApplication::palette();
        const QColor colour = mode == QIcon::Disabled ? pal.color(QPalette::Disabled, QPalette::WindowText)
                            : mode == QIcon::Selected ? pal.color(QPalette::HighlightedText)
                            : tint_.isValid()         ? tint_
                                                      : pal.color(QPalette::WindowText);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(image.rect(), colour);
        p.end();

        QPixmap pm = QPixmap::fromImage(image);
        pm.setDevicePixelRatio(dpr);
        return pm;
    }

    QString name_;
    Drawing drawing_;
    QColor tint_;
};

}   // namespace

QIcon icon(const QString& name, Drawing drawing, const QColor& tint)
{
    if (!exists(name)) { return {}; }
    return QIcon(new GlyphEngine(name, drawing, tint));
}

QToolButton* mark(const QString& name, const QSize& size, QWidget* parent, const QColor& tint)
{
    auto* b = new QToolButton(parent);
    b->setAutoRaise(true);
    b->setFocusPolicy(Qt::NoFocus);
    b->setAttribute(Qt::WA_TransparentForMouseEvents);
    b->setIcon(icon(name, Drawing::BySize, tint));
    b->setIconSize(size);
    return b;
}

bool exists(const QString& name)
{
    return QFile::exists(path(name, "small")) && QFile::exists(path(name, "medium")) && QFile::exists(path(name, "large"));
}

QIcon appIcon()
{
    return QIcon(QStringLiteral(":/icons/inkwyn-256.png"));
}

}   // namespace GlyphIcon
