// diagnostic_log_smoke.cpp — SP-185 / T-0612: the diagnostic log. Read back FROM THE FILE.
//
// ✅ What it proves ([SP-185] AC2):
//   • the log is written where Q1 says, headed by the build, and Qt's own warnings land in it;
//   • a DELIBERATE STALL of the GUI thread is reported by the watchdog — with what was running and the last command key —
//     and its end is reported too;
//   • a slow span writes one line; a fast one writes nothing;
//   • the writer's typing is never written (keys only as "typing"), while a command chord is;
//   • a second session keeps the first as scrivi.log.1, ended "session ended";
//   • mode `crash` (run by the .sh as its own process): a real bad memory access writes "crashed", what was running and a
//     backtrace before the process dies.
//   argv[1] = "log" | "crash"; $XDG_STATE_HOME chooses the folder; Qt offscreen (set by the script).

#include "DiagnosticLog.hpp"

#include <QApplication>
#include <QFile>
#include <QLineEdit>
#include <QRegularExpression>
#include <QTest>

#include <chrono>
#include <cstdio>
#include <thread>

namespace {

int failures = 0;

void check(bool cond, const QString& what)
{
    if (!cond) {
        std::printf("FAIL: %s\n", qUtf8Printable(what));
        ++failures;
    }
}

QString readLog(const QString& path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

// Blocks the GUI thread: the stall a writer sees as a frozen window.
void blockGui(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

}   // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    const QString mode = argc > 1 ? QString::fromUtf8(argv[1]) : QStringLiteral("log");

    if (mode == QStringLiteral("crash")) {
        DiagnosticLog::start(QStringLiteral("smoke build (crash)"));
        const DiagnosticLog::Span span("about to crash", DiagnosticLog::Span::Mode::Always);
        volatile int* nowhere = nullptr;
        *nowhere = 1;   // a real SIGSEGV, as a defect would raise it
        return 0;
    }

    // ── 1. Written where Q1 says, headed by the build; Qt's warnings land in it ──────────────────────────────────────────
    DiagnosticLog::start(QStringLiteral("smoke build 1"));
    check(DiagnosticLog::isStarted(), QStringLiteral("the log started"));
    check(DiagnosticLog::filePath() == qEnvironmentVariable("XDG_STATE_HOME") + QStringLiteral("/scrivi/scrivi.log"),
          QStringLiteral("the log is $XDG_STATE_HOME/scrivi/scrivi.log (%1)").arg(DiagnosticLog::filePath()));
    qWarning("a warning from Qt's own channel");
    QString log = readLog(DiagnosticLog::filePath());
    check(log.contains(QStringLiteral("==== Scrivi log — smoke build 1")), QStringLiteral("headed by the build"));
    check(log.contains(QStringLiteral("qt warning: a warning from Qt's own channel")), QStringLiteral("Qt's warning is logged"));

    // ── 2. The writer's typing is never written; a command chord is ──────────────────────────────────────────────────
    QLineEdit field;
    field.show();
    QTest::qWaitForWindowExposed(&field);
    QTest::keyClicks(&field, QStringLiteral("Villefort"));
    {
        const DiagnosticLog::Span typingStall("stall after typing");
        blockGui(600);
    }
    QTest::qWait(400);
    QTest::keyClick(&field, Qt::Key_C, Qt::ControlModifier);
    QTest::qWait(150);

    // ── 3. A deliberate stall: reported, with what was running and the last input, and its end ───────────────────────
    {
        const DiagnosticLog::Span outer("deliberate stall");
        const DiagnosticLog::Span inner("inner step");
        blockGui(900);
    }
    QTest::qWait(400);   // the heartbeat resumes; the watchdog reports the recovery
    log = readLog(DiagnosticLog::filePath());
    check(log.contains(QStringLiteral("the window is not answering")), QStringLiteral("the stall is reported"));
    check(log.contains(QStringLiteral("running: deliberate stall (")) && log.contains(QStringLiteral(" > inner step (")),
          QStringLiteral("the stall names what was running, outermost first"));
    check(log.contains(QStringLiteral("last input: Ctrl+C")), QStringLiteral("the stall names the last command key"));
    check(log.contains(QStringLiteral("the window is answering again, after")), QStringLiteral("the recovery is reported"));
    check(log.contains(QStringLiteral("slow: inner step took")), QStringLiteral("a slow span writes a line"));
    check(log.contains(QStringLiteral("key Ctrl+C")), QStringLiteral("a command chord is logged"));
    check(log.contains(QStringLiteral("running: stall after typing (")) && log.contains(QStringLiteral("last input: typing (")),
          QStringLiteral("a stall after typing names the input as \"typing\""));
    check(!log.contains(QStringLiteral("Villefort")) && !log.contains(QStringLiteral("Villef")),
          QStringLiteral("⛔ the writer's typing is NOT in the log"));
    static const QRegularExpression typedKey(QStringLiteral("(key|last input:) [A-Za-z]( |$)"), QRegularExpression::MultilineOption);
    check(!log.contains(typedKey), QStringLiteral("⛔ no typed letter is in the log, even one at a time"));

    // ── 4. A fast span writes nothing; no stall without one ──────────────────────────────────────────────────────────
    const int before = int(readLog(DiagnosticLog::filePath()).size());
    { const DiagnosticLog::Span quick("quick step"); }
    QTest::qWait(400);
    log = readLog(DiagnosticLog::filePath());
    check(!log.mid(before).contains(QStringLiteral("quick step")), QStringLiteral("a fast span writes nothing"));
    check(!log.mid(before).contains(QStringLiteral("not answering")), QStringLiteral("an answering window is not reported"));

    // ── 5. Two sessions kept: the first becomes scrivi.log.1, ended cleanly ──────────────────────────────────────────
    DiagnosticLog::stop(0);
    DiagnosticLog::start(QStringLiteral("smoke build 2"));
    const QString first = readLog(DiagnosticLog::filePath() + QStringLiteral(".1"));
    check(first.contains(QStringLiteral("smoke build 1")) && first.contains(QStringLiteral("session ended (exit code 0)")),
          QStringLiteral("the first session is kept as scrivi.log.1, ended"));
    check(readLog(DiagnosticLog::filePath()).contains(QStringLiteral("smoke build 2")), QStringLiteral("the second session is the log"));
    DiagnosticLog::stop(0);

    if (failures == 0) {
        std::printf("diagnostic-log-ok\n");
        return 0;
    }
    std::printf("%d failure(s)\n", failures);
    return 1;
}
