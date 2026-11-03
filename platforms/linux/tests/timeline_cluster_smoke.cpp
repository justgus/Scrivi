// timeline_cluster_smoke — headless co-located-dot clustering check for EP-025
// (SP-084, T-0349, AC6b). No real mouse/paint — drives TimelinePanel's clustering pass
// directly via its test hooks (aggregateCountForTest / largestAggregateSizeForTest).
//
// The clustering pass cuts the story into equal story-time slices, as many as aggregates
// fit across the zoomed width ([I-0338], Apple's [I-0208] rule), so:
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
#include <QMouseEvent>
#include <QScrollArea>

#include <cmath>
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
    //        separate when zoomed in far enough.
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

    // SP-173 — Apple's I-0211: VISIBLE imported timelines FRAME the main timeline (dumas: scenes span ~49 days, imported
    // history spans centuries; without the frame every imported event fell off the strip).
    {
        QList<TimelinePanel::Dot> dots;
        dots.append(scene(QStringLiteral("s1"), 0));
        dots.append(scene(QStringLiteral("s2"), 3'600'000));
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        panel.setHistoricalEvents({});
        panel.setImportedTimelines({});
        qint64 lo = 0, hi = 0;
        panel.storyTimeWindow(lo, hi);
        const qint64 sceneLo = lo, sceneHi = hi;

        TimelinePanel::ImportedRow row;
        row.timelineID = QStringLiteral("history");
        row.visible = true;
        row.events.append({QStringLiteral("Before"), -1'000'000'000'000LL});
        row.events.append({QStringLiteral("After"),   7'000'000'000'000LL});
        panel.setImportedTimelines({row});
        panel.storyTimeWindow(lo, hi);
        check(lo <= -1'000'000'000'000LL && hi >= 7'000'000'000'000LL,
              "I-0211: a visible imported timeline frames the window (its earliest and latest events)");

        row.visible = false;
        panel.setImportedTimelines({row});
        panel.storyTimeWindow(lo, hi);
        check(lo == sceneLo && hi == sceneHi, "I-0211: hiding the row narrows the frame back to the scenes");
    }

    // T-0641 — the time scale (the same cases as Apple's TimelineScaleTests).
    {
        constexpr qint64 kHour = 3600000LL, kDay = 86400000LL;
        check(TimelinePanel::scaleLabel(0, kDay) == QStringLiteral("Day 1"), "scale: Story Open is Day 1");
        check(TimelinePanel::scaleLabel(2 * kDay, kDay) == QStringLiteral("Day 3"), "scale: two days on is Day 3");
        check(TimelinePanel::scaleLabel(-kDay, kDay) == QStringLiteral("Day -1"), "scale: the day before is Day -1");
        check(TimelinePanel::scaleLabel(2 * kDay + 14 * kHour, kHour) == QStringLiteral("Day 3 14:00"),
              "scale: hour ticks carry the time");
        check(TimelinePanel::scaleLabel(365 * kDay, 365 * kDay) == QStringLiteral("Year 2"), "scale: a year on is Year 2");

        // One day across 900 px: 90 px apart needs a 2.4 h step, so 3 h ticks, all inside the window.
        const auto ticks = TimelinePanel::scaleTicks(kHour / 2, kDay, 900);
        check(!ticks.isEmpty() && ticks.first().ms == 3 * kHour, "scale: the first tick is 03:00");
        bool spaced = true, inside = true;
        for (int i = 0; i < ticks.size(); ++i) {
            if (i > 0 && ticks.at(i).ms - ticks.at(i - 1).ms != 3 * kHour) { spaced = false; }
            if (ticks.at(i).ms < kHour / 2 || ticks.at(i).ms > kHour / 2 + kDay) { inside = false; }
        }
        check(spaced, "scale: ticks are 3 h apart");
        check(inside, "scale: every tick is inside the window");

        // ~240 years (the Dumas fixture's framed span) across 1,000 px.
        const auto huge = TimelinePanel::scaleTicks(0, 240 * 365 * kDay, 1000);
        check(!huge.isEmpty() && huge.size() < 20 && huge.first().label == QStringLiteral("Year 1"),
              "scale: a huge span still gets a few labels");
    }

    // T-0640 — zoom to the active scene: its neighbours separate and it sits mid-strip.
    {
        constexpr qint64 kDay = 86400000LL;
        QList<TimelinePanel::Dot> dots;
        // B mid-span, so centring it is not clamped by an edge.
        dots << scene(QStringLiteral("A"), 0)
             << scene(QStringLiteral("B"), 180 * kDay)
             << scene(QStringLiteral("C"), 182 * kDay)
             << scene(QStringLiteral("Z"), 1 * kYear);
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        panel.setHistoricalEvents({});
        panel.setImportedTimelines({});
        panel.setViewState(1.0, 0.0);
        check(panel.aggregateCountForTest() == 1, "zoom-to-scene: B starts clustered");

        double savedZoom = 0, savedPan = -1;
        QObject::connect(&panel, &TimelinePanel::viewStateChanged, [&](double z, double p) { savedZoom = z; savedPan = p; });
        panel.setActiveScene(QStringLiteral("B"));
        panel.zoomToActiveScene();
        check(panel.aggregateCountForTest() == 0, "zoom-to-scene: B and C are single dots");
        qint64 lo = 0, hi = 0;
        panel.storyTimeWindow(lo, hi);
        const double bFrac = double(180 * kDay - lo) / double(hi - lo);
        const double centre = panel.panFraction() + 0.5 / panel.zoomFactor();
        check(std::abs(centre - bFrac) < 1e-6, "zoom-to-scene: B is centred");
        check(savedZoom == panel.zoomFactor() && savedPan == panel.panFraction(),
              "zoom-to-scene: the new view is saved (T-0338)");
    }

    // [I-0338] — a dense story (dumas: 1,175 scenes) never chains into ONE aggregate (R2), zooming in never makes the largest
    // aggregate larger (R3), and zoom to scene leaves the active scene a single dot.
    {
        constexpr qint64 kHour = 3600000LL;
        QList<TimelinePanel::Dot> dots;
        for (int k = 0; k < 1175; ++k) {
            dots << scene(QStringLiteral("S%1").arg(k), k * kHour);   // ~1 px apart at zoom 1: the old rule chained them all
        }
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        panel.setViewState(1.0, 0.0);
        const int atOne = panel.aggregateCountForTest();
        check(atOne >= 20, "I-0338: 1,175 dense scenes form many aggregates at zoom 1, not one");
        int largest = panel.largestAggregateSizeForTest();
        bool monotonic = true;
        for (double z : {2.0, 5.0, 20.0, 100.0}) {
            panel.setViewState(z, 0.0);
            const int now = panel.largestAggregateSizeForTest();
            monotonic = monotonic && now <= largest;
            largest = now;
        }
        check(monotonic, "I-0338: zooming in never makes the largest aggregate larger (R3)");
        panel.setActiveScene(QStringLiteral("S600"));
        panel.zoomToActiveScene();
        bool single = true;
        for (int a = 0; a < panel.aggregateCountForTest(); ++a) {
            single = single && !panel.aggregateHoldsSceneForTest(a, 600);
        }
        check(single, "I-0338: zoom to scene leaves the active scene a single dot in a dense story");
    }

    // [I-0340] — imported history (centuries) frames the strip: zoom to scene still separates scenes an hour apart (a fixed
    // 500× left ~78 in each aggregate on dumas). Apple's rule: max zoom = span ÷ the closest two dots, floor 1 h.
    {
        constexpr qint64 kHour = 3600000LL;
        QList<TimelinePanel::Dot> dots;
        for (int k = 0; k < 1175; ++k) {
            dots << scene(QStringLiteral("S%1").arg(k), k * kHour);
        }
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        TimelinePanel::HistDot far;
        far.eventID = QStringLiteral("ancient");
        far.title = QStringLiteral("The founding");
        far.offsetMs = -500 * kYear;
        panel.setHistoricalEvents({far});
        panel.setViewState(1.0, 0.0);
        panel.setActiveScene(QStringLiteral("S600"));
        panel.zoomToActiveScene();
        bool single = true;
        for (int a = 0; a < panel.aggregateCountForTest(); ++a) {
            single = single && !panel.aggregateHoldsSceneForTest(a, 600);
        }
        check(single, "I-0340: zoom to scene separates hour-apart scenes in a strip framed by 500 years of history");
        panel.setHistoricalEvents({});
    }

    // [I-0286] — hovering an aggregate opens its members in a WINDOW of their own (never clipped by the strip), bounded:
    // beyond 12 members a scrollable grid at most 260 px tall.
    {
        QList<TimelinePanel::Dot> dots;
        for (int k = 0; k < 40; ++k) {
            dots << scene(QStringLiteral("C%1").arg(k), 0);   // forty scenes at one story time: one aggregate at any zoom
        }
        dots << scene(QStringLiteral("Z"), 10 * kYear);
        panel.setTimeline(QStringLiteral("Story Open"), dots);
        panel.setViewState(1.0, 0.0);
        check(panel.largestAggregateSizeForTest() == 40, "I-0286: forty coincident scenes form one aggregate");
        const QPointF at(panel.aggregateXForTest(0), panel.projectRowYForTest());
        QMouseEvent move(QEvent::MouseMove, at, panel.mapToGlobal(at), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&panel, &move);
        auto* popup = panel.findChild<QWidget*>(QStringLiteral("timelineAggregatePopup"));
        check(popup != nullptr && popup->isVisible() && popup->isWindow(),
              "I-0286: hovering the aggregate opens its members in a window of their own");
        auto* scroll = popup != nullptr ? popup->findChild<QScrollArea*>() : nullptr;
        check(scroll != nullptr && scroll->height() <= 260, "I-0286: forty members are a scrollable grid at most 260 px tall");
    }

    if (failures > 0) {
        std::fprintf(stderr, "timeline_cluster_smoke: %d check(s) FAILED\n", failures);
        return 1;
    }
    std::fprintf(stderr, "PASS: co-located dot clustering + zoom-resolve.\n");
    return 0;
}
