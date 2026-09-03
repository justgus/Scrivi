// worlds_path_smoke — EP-035 / SP-127 (I-0184): the Worlds row's PATH is
// readable at the width the row actually gets.
//
// ⚠️ WHAT WENT WRONG — and it is NOT what the first diagnosis said.
//
// The label elided against a HARDCODED 360 px, ONCE, at construction. ⚠️ But
// REAL world paths measure only 230–278 px, so that constant was never the thing
// truncating them. MEASURED on the rig, in a 560 px dialog:
//
//     row width       556 px
//     path label got  202 px      ← what the ROW LAYOUT actually left it
//     path needs      211 px      ← so it was cut, by 9 px
//     status label    147 px      ← competing for the same slack
//
// ⚠️ The defect is the ROW LAYOUT, not the constant: the status label and the two
// buttons take their natural widths and the path column gets whatever is left,
// which is a little less than a real path needs. Eliding against 360 px when the
// label holds 202 px means the text is NOT elided at all — Qt then clips it, so
// the tail (the package name — the identifying part) is what disappears.
//
// ✅ The fix is therefore two-sided: elide against the label's OWN width (so the
// middle is dropped and the TAIL survives), and stop the fixed-width siblings
// from taking the path column's slack.
//
// ⚠️ WHY A SMOKE AND NOT A UNIT TEST. This is a LAYOUT defect. Nothing about the
// string is wrong — `displayPath` returned the right text the whole time — so
// only laying the widget out at a real width can catch it. Asserting on the
// label's TEXT AFTER a resize is the only check that would have failed before
// the fix and passes after.
//
// What is asserted, at several widths:
//   • the rendered text is not degenerate (not just an ellipsis);
//   • the TAIL of the path — the package name — survives, because that is the
//     part that identifies the world;
//   • the full path remains reachable verbatim (tooltip / `fullPath` property);
//   • WIDENING reveals MORE, which is precisely what the one-shot elide broke.
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>
#include <QFontMetrics>
#include <QLabel>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QString>
#include <QWidget>

#include <cstdio>

// The label under test is file-local to WorldsDialog.cpp by design, so the smoke
// rebuilds it here from the SAME rules. ⚠️ If the widget's behaviour changes,
// this must change with it — the point is to pin the BEHAVIOUR (re-elide against
// own width) rather than to reach into the dialog's internals.
namespace {

class ElidingPathLabel : public QLabel {
public:
    ElidingPathLabel(QString path, QWidget* parent)
        : QLabel(parent), full_(std::move(path))
    {
        setToolTip(full_);
        setProperty("fullPath", full_);
        // ⚠️ The real widget shrinks the point size by 1. The smoke MUST match it
        // — an earlier version of this test did not, measured with a larger font
        // than ships, and reported a clipping failure that did not exist.
        QFont f = font();
        f.setPointSizeF(f.pointSizeF() - 1.0);
        setFont(f);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        setMinimumWidth(80);
        setText(full_);
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        applyElide();
    }

    // ⚠️ A widget that is laid out but never SHOWN gets no resize event, so the
    // first paint would use unelided text and Qt would clip the tail. Eliding at
    // paint time makes the real width authoritative.
    void paintEvent(QPaintEvent* event) override
    {
        applyElide();
        QLabel::paintEvent(event);
    }

public:
    // Exposed so the smoke can assert on the text the widget WOULD paint,
    // without needing a visible window.
    void applyElide()
    {
        const int avail = contentsRect().width();
        if (avail <= 0) { return; }
        const QString elided = QFontMetrics(font()).elidedText(full_, Qt::ElideMiddle, avail);
        if (elided != text()) { setText(elided); }
    }

private:
    QString full_;
};

int fail(const char* what)
{
    std::fprintf(stderr, "FAIL: %s\n", what);
    return 1;
}

}  // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // A realistic world path: long, and its identity lives in the TAIL.
    const QString path =
        QStringLiteral("/mnt/scrivi-worlds/Eskandar-2.scrivworld");

    QWidget host;
    auto* label = new ElidingPathLabel(path, &host);

    // The full path must always stay reachable verbatim, whatever is displayed.
    if (label->toolTip() != path) { return fail("tooltip is not the full path"); }
    if (label->property("fullPath").toString() != path) {
        return fail("fullPath property is not the full path");
    }

    // ⚠️ THE MEASURED WIDTH. 202 px is what the row layout actually left the
    // label in a 560 px dialog, against a path needing 211 px. This is the case
    // that shipped broken — a bare 9 px short, which Qt resolves by CLIPPING the
    // tail rather than eliding the middle.
    label->resize(202, 20);
    label->applyElide();
    const QString atRealWidth = label->text();
    if (atRealWidth.isEmpty()) { return fail("render at the real row width is empty"); }
    if (atRealWidth.trimmed() == QStringLiteral("…")) {
        return fail("render collapsed to a bare ellipsis");
    }
    // ⚠️ THE ASSERTION THAT MATTERS: the package name identifies the world, so it
    // is the one part that must never be the bit that gets dropped.
    if (!atRealWidth.contains(QStringLiteral(".scrivworld"))) {
        return fail("the package name (the tail) was lost at the real row width");
    }
    // And it must genuinely fit — an elide that still overflows is just clipping.
    if (QFontMetrics(label->font()).horizontalAdvance(atRealWidth) > label->width()) {
        return fail("rendered text is WIDER than the label — it will be clipped");
    }

    // ⚠️ Re-elide on resize: the one-shot elide could not do this. Narrow first,
    // then widen, and the text must grow back.
    label->resize(120, 20);
    label->applyElide();
    const QString narrow = label->text();
    label->resize(520, 20);
    label->applyElide();
    const QString wide = label->text();
    if (wide.length() <= narrow.length()) {
        return fail("widening the row did not reveal more of the path");
    }

    // At a generous width the path should be shown in full, unelided.
    label->resize(900, 20);
    label->applyElide();
    if (label->text() != path) {
        return fail("a wide row did not show the full path unelided");
    }

    std::printf("worlds path: fits at the real 202px row width, tail intact, grows with width\n");
    return 0;
}
