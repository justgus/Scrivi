#pragma once

// SP-185 / T-0612 — THE DIAGNOSTIC LOG: what the app was doing, and when the window stopped answering.
//
// ✅ [SP-185] Q1 (ruled 2026-10-17, user: "build the log as proposed"):
//   • one file, `$XDG_STATE_HOME/scrivi/scrivi.log` (default `~/.local/state/scrivi/`), always on; the previous session is
//     kept as `scrivi.log.1`, so two sessions survive;
//   • each line: local time, thread, and what started or finished, with its duration;
//   • a main-thread WATCHDOG: when the window stops answering for more than 250 ms it logs that, with what was running and
//     the last command key — [I-0307]'s symptom exactly;
//   • Qt's own warnings and fatal messages, so an exit that is not a crash still leaves its last words.
//   Help ▸ Show Log Folder opens the folder.
// ✅ Also, for [I-0308] ("the app simply went away"): a fatal signal writes "crashed", what was running and a backtrace
//    before the default action runs (a core dump still follows where the system keeps one); a clean quit writes "session
//    ended". ⚠️ A session with NEITHER line was killed outright (SIGKILL, the out-of-memory killer): nothing can log that.
// ⛔ NEVER THE WRITER'S TEXT. Keys are logged only as command chords (Ctrl+C), plain typing as "typing"; no span carries
//    manuscript text or a find query.
// ⚠️ Every line is one `write()` to an O_APPEND descriptor — unbuffered, so what was written survives a crash.
// ⚠️ Inert until `start()`: smokes that never start it pay only an atomic load per span.

#include <QString>

#include <chrono>

namespace DiagnosticLog {

// Opens the log (rotating the previous one), installs the message handler, the fatal-signal handlers, the input tracker
// and the watchdog. Call once, after QApplication exists. `header` heads the file (the build stamp).
void start(const QString& header);
// Writes "session ended" and closes the log. After `app.exec()` returns.
void stop(int exitCode);
bool isStarted();

QString folder();
QString filePath();

// One line, from any thread.
void note(const QString& text);

// A timed operation. On the GUI thread it is also what the watchdog reports as running.
// `Always` writes a begin and an end line; `IfSlow` writes one line only when it took ≥ 50 ms.
class Span
{
public:
    enum class Mode { IfSlow, Always };
    explicit Span(const char* name, Mode mode = Mode::IfSlow);
    ~Span();
    Span(const Span&) = delete;
    Span& operator=(const Span&) = delete;

private:
    const char* name_;
    Mode mode_;
    bool active_ = false;
    bool onGui_ = false;
    std::chrono::steady_clock::time_point begin_;
};

// The watchdog's thresholds (tests).
inline constexpr int kStallMs = 250;
inline constexpr int kSlowSpanMs = 50;

}   // namespace DiagnosticLog
