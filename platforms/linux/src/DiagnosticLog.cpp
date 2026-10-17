#include "DiagnosticLog.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QKeyEvent>
#include <QKeySequence>
#include <QThread>
#include <QTimer>

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include <execinfo.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace DiagnosticLog {

namespace {

using Clock = std::chrono::steady_clock;

std::atomic<bool> g_started{false};
int g_fd = -1;
std::mutex g_writeMutex;
QThread* g_gui = nullptr;
QtMessageHandler g_previousHandler = nullptr;

// The GUI thread's running spans, innermost last — what the watchdog and a crash report name.
std::mutex g_stackMutex;
std::vector<std::pair<const char*, Clock::time_point>> g_stack;
// The same, as plain text, for the crash handler (which may not lock). Racy by nature; good enough for a last word.
char g_runningText[512] = "nothing";

std::mutex g_inputMutex;
QString g_lastInput = QStringLiteral("none");
Clock::time_point g_lastInputAt;

std::atomic<long long> g_beatNs{0};
std::atomic<bool> g_stopWatchdog{false};
std::thread g_watchdog;
QTimer* g_heartbeat = nullptr;
QObject* g_inputTracker = nullptr;

long long nowNs() { return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count(); }
long long msSince(Clock::time_point t) { return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t).count(); }

QString threadLabel()
{
    if (QThread::currentThread() == g_gui) {
        return QStringLiteral("main");
    }
    return QStringLiteral("t%1").arg(long(::syscall(SYS_gettid)));
}

void writeRaw(const QByteArray& bytes)
{
    if (g_fd < 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_writeMutex);
    const char* p = bytes.constData();
    qsizetype left = bytes.size();
    while (left > 0) {
        const ssize_t n = ::write(g_fd, p, size_t(left));
        if (n <= 0) {
            return;
        }
        p += n;
        left -= n;
    }
}

void writeLine(const QString& thread, const QString& text)
{
    const QString line = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")) + QStringLiteral(" [")
                         + thread + QStringLiteral("] ") + text + QLatin1Char('\n');
    writeRaw(line.toUtf8());
}

QString runningNow()
{
    std::lock_guard<std::mutex> lock(g_stackMutex);
    if (g_stack.empty()) {
        return QStringLiteral("nothing Scrivi has marked (Qt's own event handling, layout or painting)");
    }
    QStringList parts;
    for (const auto& [name, at] : g_stack) {
        parts.append(QStringLiteral("%1 (%2 ms)").arg(QLatin1String(name)).arg(msSince(at)));
    }
    return parts.join(QStringLiteral(" > "));
}

void refreshRunningText()   // with g_stackMutex held
{
    std::string s;
    for (const auto& entry : g_stack) {
        if (!s.empty()) {
            s += " > ";
        }
        s += entry.first;
    }
    if (s.empty()) {
        s = "nothing";
    }
    std::strncpy(g_runningText, s.c_str(), sizeof g_runningText - 1);
    g_runningText[sizeof g_runningText - 1] = '\0';
}

QString lastInput()
{
    std::lock_guard<std::mutex> lock(g_inputMutex);
    if (g_lastInput == QStringLiteral("none")) {
        return g_lastInput;
    }
    return QStringLiteral("%1 (%2 ms before)").arg(g_lastInput).arg(msSince(g_lastInputAt));
}

// ── Qt's own messages ─────────────────────────────────────────────────────────────────────────────────────────────────

void onMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (g_started.load()) {
        static const char* kinds[] = {"debug", "warning", "critical", "fatal", "info"};
        const int k = int(type);
        writeLine(threadLabel(), QStringLiteral("qt %1: %2").arg(QLatin1String(k >= 0 && k <= 4 ? kinds[k] : "message"), message));
    }
    if (g_previousHandler != nullptr) {
        g_previousHandler(type, context, message);   // stderr as before (and qFatal's abort)
    }
}

// ── Fatal signals ─────────────────────────────────────────────────────────────────────────────────────────────────────

void writeSafe(const char* s)
{
    if (g_fd >= 0) {
        (void)!::write(g_fd, s, std::strlen(s));
    }
}

const char* signalName(int sig)
{
    switch (sig) {
    case SIGSEGV: return "SIGSEGV, a bad memory access";
    case SIGBUS: return "SIGBUS, a bad memory access";
    case SIGFPE: return "SIGFPE, an arithmetic fault";
    case SIGILL: return "SIGILL, an illegal instruction";
    case SIGABRT: return "SIGABRT, an abort";
    case SIGTERM: return "SIGTERM, asked to stop";
    default: return "a fatal signal";
    }
}

void onFatalSignal(int sig)
{
    // ⚠️ Async-signal context: write(), backtrace_symbols_fd() and static text only.
    char number[16];
    std::snprintf(number, sizeof number, "%d", sig);
    if (sig == SIGTERM) {
        writeSafe("*** received signal ");
        writeSafe(number);
        writeSafe(" (SIGTERM, asked to stop) — Scrivi did not get to quit normally\n");
    } else {
        writeSafe("*** crashed: signal ");
        writeSafe(number);
        writeSafe(" (");
        writeSafe(signalName(sig));
        writeSafe(")\n*** running on the main thread: ");
        writeSafe(g_runningText);
        writeSafe("\n*** backtrace of the crashing thread:\n");
        void* frames[64];
        const int n = ::backtrace(frames, 64);
        if (g_fd >= 0) {
            ::backtrace_symbols_fd(frames, n, g_fd);
        }
        writeSafe("*** end of crash report\n");
    }
    std::signal(sig, SIG_DFL);   // the default action follows: the exit, and a core dump where the system keeps one
    std::raise(sig);
}

void installSignalHandlers()
{
    // A separate stack, so a stack overflow can still be reported.
    static char altStack[64 * 1024];
    stack_t ss{};
    ss.ss_sp = altStack;
    ss.ss_size = sizeof altStack;
    ::sigaltstack(&ss, nullptr);

    void* warm[2];
    (void)::backtrace(warm, 2);   // loads the unwinder now, not inside the handler

    struct sigaction sa{};
    sa.sa_handler = onFatalSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK | SA_RESETHAND;
    for (int sig : {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT, SIGTERM}) {
        ::sigaction(sig, &sa, nullptr);
    }
}

// ── Input: the last command key, never the writer's text ─────────────────────────────────────────────────────────────

class InputTracker : public QObject
{
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        // ⚠️ An unhandled event is offered to each parent in turn: count it once.
        if (event == lastEvent_) {
            return QObject::eventFilter(watched, event);
        }
        QString what;
        bool chord = false;
        switch (event->type()) {
        case QEvent::KeyPress: {
            const auto* k = static_cast<QKeyEvent*>(event);
            const Qt::KeyboardModifiers command = k->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
            if (command != Qt::NoModifier) {
                what = QKeySequence(k->keyCombination()).toString();
                chord = !k->isAutoRepeat();
            } else if (!k->text().isEmpty() && k->text().at(0).isPrint()) {
                what = QStringLiteral("typing");
            } else {
                what = QKeySequence(k->keyCombination()).toString();   // Return, Backspace, Shift+Down …
            }
            break;
        }
        case QEvent::MouseButtonPress: what = QStringLiteral("mouse press"); break;
        case QEvent::MouseButtonDblClick: what = QStringLiteral("double click"); break;
        case QEvent::Wheel: what = QStringLiteral("scroll wheel"); break;
        default: return QObject::eventFilter(watched, event);
        }
        lastEvent_ = event;
        {
            std::lock_guard<std::mutex> lock(g_inputMutex);
            g_lastInput = what;
            g_lastInputAt = Clock::now();
        }
        if (chord) {
            writeLine(QStringLiteral("main"), QStringLiteral("key %1").arg(what));
        }
        return QObject::eventFilter(watched, event);
    }

private:
    const QEvent* lastEvent_ = nullptr;
};

// ── The watchdog ──────────────────────────────────────────────────────────────────────────────────────────────────────

void watch()
{
    bool stalled = false;
    long long stallBeganNs = 0;
    long long nextReportNs = 0;
    while (!g_stopWatchdog.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const long long now = nowNs();
        const long long beat = g_beatNs.load();
        const long long lagMs = (now - beat) / 1000000;
        if (!stalled && lagMs > kStallMs) {
            stalled = true;
            stallBeganNs = beat;
            nextReportNs = now + 2000LL * 1000000;
            writeLine(QStringLiteral("watchdog"), QStringLiteral("⚠ the window is not answering (%1 ms so far); running: %2; last input: %3")
                                                      .arg(lagMs).arg(runningNow(), lastInput()));
        } else if (stalled && lagMs > kStallMs && now >= nextReportNs) {
            nextReportNs = now + 5000LL * 1000000;
            writeLine(QStringLiteral("watchdog"), QStringLiteral("⚠ still not answering (%1 ms); running: %2").arg(lagMs).arg(runningNow()));
        } else if (stalled && lagMs <= kStallMs) {
            stalled = false;
            writeLine(QStringLiteral("watchdog"),
                      QStringLiteral("the window is answering again, after %1 ms").arg((beat - stallBeganNs) / 1000000));
        }
    }
}

}   // namespace

QString folder()
{
    QString state = qEnvironmentVariable("XDG_STATE_HOME");
    if (state.isEmpty()) {
        state = QDir::homePath() + QStringLiteral("/.local/state");
    }
    return state + QStringLiteral("/scrivi");
}

QString filePath() { return folder() + QStringLiteral("/scrivi.log"); }

bool isStarted() { return g_started.load(); }

void start(const QString& header)
{
    if (g_started.load() || QCoreApplication::instance() == nullptr) {
        return;
    }
    g_gui = QCoreApplication::instance()->thread();
    QDir().mkpath(folder());
    // Two sessions kept: the previous log becomes scrivi.log.1. (POSIX rename, not QFile: this is not project data.)
    const QByteArray path = QFile::encodeName(filePath());
    const QByteArray previous = path + ".1";
    ::rename(path.constData(), previous.constData());
    g_fd = ::open(path.constData(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (g_fd < 0) {
        std::fprintf(stderr, "Scrivi: could not open the diagnostic log %s\n", path.constData());
        return;
    }
    g_started.store(true);
    writeLine(QStringLiteral("main"), QStringLiteral("==== Scrivi log — %1 — pid %2").arg(header).arg(long(::getpid())));
    writeLine(QStringLiteral("main"), QStringLiteral("(a session that ends with neither \"session ended\" nor \"crashed\" was killed "
                                                     "outright: SIGKILL, or the out-of-memory killer)"));

    g_previousHandler = qInstallMessageHandler(onMessage);
    installSignalHandlers();

    g_inputTracker = new InputTracker(QCoreApplication::instance());
    QCoreApplication::instance()->installEventFilter(g_inputTracker);

    g_beatNs.store(nowNs());
    g_heartbeat = new QTimer(QCoreApplication::instance());
    g_heartbeat->setInterval(100);
    QObject::connect(g_heartbeat, &QTimer::timeout, [] { g_beatNs.store(nowNs()); });
    g_heartbeat->start();
    g_stopWatchdog.store(false);
    g_watchdog = std::thread(watch);
}

void stop(int exitCode)
{
    if (!g_started.load()) {
        return;
    }
    g_stopWatchdog.store(true);
    if (g_watchdog.joinable()) {
        g_watchdog.join();
    }
    delete g_heartbeat;
    g_heartbeat = nullptr;
    if (g_inputTracker != nullptr) {
        QCoreApplication::instance()->removeEventFilter(g_inputTracker);
        delete g_inputTracker;
        g_inputTracker = nullptr;
    }
    writeLine(QStringLiteral("main"), QStringLiteral("session ended (exit code %1)").arg(exitCode));
    qInstallMessageHandler(g_previousHandler);
    for (int sig : {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT, SIGTERM}) {
        std::signal(sig, SIG_DFL);
    }
    g_started.store(false);
    const int fd = g_fd;
    {
        std::lock_guard<std::mutex> lock(g_writeMutex);
        g_fd = -1;
    }
    ::close(fd);
}

void note(const QString& text)
{
    if (g_started.load()) {
        writeLine(threadLabel(), text);
    }
}

Span::Span(const char* name, Mode mode) : name_(name), mode_(mode)
{
    if (!g_started.load()) {
        return;
    }
    active_ = true;
    begin_ = Clock::now();
    onGui_ = QThread::currentThread() == g_gui;
    if (onGui_) {
        std::lock_guard<std::mutex> lock(g_stackMutex);
        g_stack.emplace_back(name_, begin_);
        refreshRunningText();
    }
    if (mode_ == Mode::Always) {
        writeLine(threadLabel(), QStringLiteral("begin %1").arg(QLatin1String(name_)));
    }
}

Span::~Span()
{
    if (!active_) {
        return;
    }
    const long long ms = msSince(begin_);
    if (onGui_) {
        std::lock_guard<std::mutex> lock(g_stackMutex);
        if (!g_stack.empty()) {
            g_stack.pop_back();
        }
        refreshRunningText();
    }
    if (mode_ == Mode::Always) {
        writeLine(threadLabel(), QStringLiteral("end %1 (%2 ms)").arg(QLatin1String(name_)).arg(ms));
    } else if (ms >= kSlowSpanMs) {
        writeLine(threadLabel(), QStringLiteral("slow: %1 took %2 ms").arg(QLatin1String(name_)).arg(ms));
    }
}

}   // namespace DiagnosticLog
