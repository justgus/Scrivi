#pragma once

// EP-054 S5b (SP-214, T-0675) — the core's print page (`document: "print"`) paginated into a PDF by QTextDocument and
// QPdfWriter (Qt GUI). Apple: `ManuscriptPDF.swift` (WebKit).
//
// Rulings: Q1 — the page numbers are drawn here, centred in the bottom margin. Q2 — QTextDocument draws in-process, so the
// bundled faces registered with addApplicationFont are the PDF's, embedded. Q3 — Letter where the region measures in US
// units, A4 elsewhere (no QtPrintSupport, so no printer to ask); 1-inch margins. Q4 — the first chapter page is page 1; the
// title page shows no number. Safe off the GUI thread: QPainter on a QPdfWriter and QTextDocument are reentrant.
// ⚠️ QTextDocument reads CSS lengths in px only (a pt margin or indent is dropped), so the print page's lengths are px and the
// writer works at 96 dpi, where a px is the 1/96 inch it is in WebKit; font sizes stay pt.

#include <QAbstractTextDocumentLayout>
#include <QFont>
#include <QLocale>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSaveFile>
#include <QTextBlock>
#include <QTextCursor>
#include <QString>
#include <QTextDocument>

// Writes the PDF of `html` to `path` (through a file beside it, renamed into place). Returns an error message, or empty.
inline QString writeManuscriptPdf(const QString& html, bool titlePage, const QString& path)
{
    QSaveFile file(path);   // boundary-ok: Export Manuscript…, the file the writer chose
    if (!file.open(QIODevice::WriteOnly)) {
        return file.errorString();
    }
    constexpr double margin = 96;   // one inch, in CSS px
    QPdfWriter writer(&file);
    writer.setResolution(96);       // a device unit is a CSS px
    writer.setPageSize(QPageSize(QLocale().measurementSystem() == QLocale::ImperialUSSystem ? QPageSize::Letter
                                                                                             : QPageSize::A4));
    writer.setPageMargins(QMarginsF(0, 0, 0, 0));
    const QSizeF paper(writer.width(), writer.height());
    const QSizeF text(paper.width() - 2 * margin, paper.height() - 2 * margin);

    QTextDocument doc;
    doc.documentLayout()->setPaintDevice(&writer);
    doc.setDocumentMargin(0);
    doc.setPageSize(text);   // QTextDocument then paginates, honouring page-break-before
    doc.setHtml(html);
    // A heading carries Qt's relative size step as well as the page's pt size, and the step wins: drop it.
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            QTextCharFormat f = it.fragment().charFormat();
            if (f.hasProperty(QTextFormat::FontSizeAdjustment) && f.hasProperty(QTextFormat::FontPointSize)) {
                f.clearProperty(QTextFormat::FontSizeAdjustment);
                QTextCursor c(&doc);
                c.setPosition(it.fragment().position());
                c.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);
                c.setCharFormat(f);
            }
        }
    }

    QPainter painter;
    if (!painter.begin(&writer)) {
        return QStringLiteral("could not draw the PDF");
    }
    QFont numberFont = doc.begin().begin().fragment().charFormat().font();   // the page's face, as the text resolved it
    numberFont.setBold(false);
    numberFont.setPointSizeF(10);
    for (int i = 0; i < doc.pageCount(); ++i) {
        if (i > 0) {
            writer.newPage();
        }
        painter.save();
        painter.translate(margin, margin - i * text.height());
        doc.drawContents(&painter, QRectF(QPointF(0, i * text.height()), text));
        painter.restore();
        if (!titlePage || i > 0) {
            painter.setFont(numberFont);
            painter.drawText(QRectF(0, paper.height() - margin, paper.width(), margin / 2), Qt::AlignHCenter | Qt::AlignBottom,
                             QString::number(titlePage ? i : i + 1));
        }
    }
    painter.end();
    return file.commit() ? QString() : file.errorString();
}
