// timeline_cluster_smoke — headless co-located-dot clustering check for EP-025
// (SP-084, T-0349, AC6b). No real mouse/paint — drives TimelinePanel's clustering pass
// directly via its test hooks (aggregateCountForTest / largestAggregateSizeForTest).
//
// The clustering pass groups project-row members (scene + historical dots) whose
// SCREEN-x are within one dot-diameter at the current zoom (FR-032), so:
//   1. Several dots at (nearly) the same story-time collapse into ONE aggregate whose
//      member count = the number of co-located dots.
//   2. Zooming in far enough spreads their pixels past the threshold → the aggregate
//      RESOLVES into singletons (zero aggregates) — the zoom-resolve contract.
//   3. Well-separated dots never cluster (zero aggregates at zoom 1).
//   4. Historical events participate in the same aggregate as scenes.
//
// Uses the Qt "offscreen" platform (a QApplication is created; no window/display). The
// panel is resized to a known width so xForOffset is well-defined.
//
// Exit 0 on success; non-zero with a FAIL line on any mismatch.

#include <QApplication>

#include <cstdio>

#include "TimelinePanel.hpp"

namespace {
int failures = 0;
void check(bool cond, const char* what)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

TimelinePanel::Dot scene(const QString& id, qint64 offsetMs)
{
    TimelinePanel::Dot d;
    d.sceneID    = id;
    d.title      = id;
    d.offsetMs   = offsetMs;
    d.durationMs = 3600000;   // 1h
    return d;
}
} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    TimelinePanel panel;
    panel.resize(800, 120);   // fixed width so xForOffset is deterministic

    constexpr qint64 kYear = 31536000000LL;

    // --- 1. Three co-located scenes (same offset) → one aggregate of 3 ----
    {
        QList<TimelinePanel::Dot> dots;
        dots << scene(QStringLiteral("A"), 0)
             << scene(QStringLiteral("B"), 0)
             << scene(QStringLiteral("C"), 0)
             << scene(QStringLiteral("Z"), 10 * kYear);   // far outlier → its own singleton
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        check(panel.aggregateCountForTest() == 1,
              "three coincident scenes form exactly one aggregate");
        check(panel.largestAggregateSizeForTest() == 3,
              "the aggregate has 3 members");
    }

    // --- 2. Zoom-resolve: dots a small distance apart cluster at zoom 1, ---
    //        separate when zoomed in far enough (within the [1,500] zoom clamp).
    {
        // Three scenes 2 days apart over a ~1-year window. At zoom 1 their pixel spacing
        // (~4px in an 800px strip) is under one dot-diameter (12px) → clustered; at zoom
        // ~8 the spacing exceeds the threshold → resolved into singletons.
        constexpr qint64 kDay = 86400000LL;
        QList<TimelinePanel::Dot> dots;
        dots << scene(QStringLiteral("A"), 0)
             << scene(QStringLiteral("B"), 2 * kDay)
             << scene(QStringLiteral("C"), 4 * kDay)
             << scene(QStringLiteral("Z"), 1 * kYear);   // sets the ~1-year window
        panel.setTimeline(QStringLiteral("Story Open"), dots);

        panel.setViewState(1.0, 0.0);
        check(panel.aggregateCountForTest() == 1,
              "at zoom 1, the three near scenes cluster into one aggregate");

        // Zoom in, keeping the cluster at the left edge (panFraction 0).
        panel.setViewState(10.0, 0.0);
        check(panel.aggregateCountForTest() == 0,
              "zooming in resolves the cluster into singletons (zoom-resolve)");
    }

    // --- 3. Well-separated scenes never cluster ---------------------------
    {
        QList<TimelinePanel::Dot> dots;
        dots << scene(QStringLiteral("A"), 0)
             << scene(QStringLiteral("B"), 3 * kYear)
             << scene(QStringLiteral("C"), 6 * kYear);
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        panel.setViewState(1.0, 0.0);
        check(panel.aggregateCountForTest() == 0,
              "evenly-spread scenes across the window do not cluster");
    }

    // --- 4. A historical event co-located with scenes joins the aggregate -
    {
        QList<TimelinePanel::Dot> dots;
        dots << scene(QStringLiteral("A"), 0)
             << scene(QStringLiteral("B"), 0)
             << scene(QStringLiteral("Z"), 10 * kYear);
        panel.setTimeline(QStringLiteral("Story Open"), dots);

        QList<TimelinePanel::HistDot> hist;
        TimelinePanel::HistDot h;
        h.eventID  = QStringLiteral("h1");
        h.title    = QStringLiteral("Founding");
        h.offsetMs = 0;   // same spot as A and B
        hist << h;
        panel.setHistoricalEvents(hist);
        panel.setViewState(1.0, 0.0);

        check(panel.aggregateCountForTest() == 1,
              "a historical event co-located with two scenes forms one aggregate");
        check(panel.largestAggregateSizeForTest() == 3,
              "the aggregate includes the historical event (3 members)");
    }

    if (failures > 0) {
        std::fprintf(stderr, "timeline_cluster_smoke: %d check(s) FAILED\n", failures);
        return 1;
    }
    std::fprintf(stderr, "PASS: co-located dot clustering + zoom-resolve.\n");
    return 0;
}
