#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#include <functional>
#include <utility>

// AsyncCall — run a blocking ScriviCore call OFF the UI thread, with a timeout.
//
// ⚠️ WHY THIS EXISTS (I-0193 / I-0195, SP-124).
//
// Every ScriviCore call in this app ran synchronously on the UI thread. That was
// invisible while worlds lived on local disk and calls returned in ~0.09 s. On
// 2026-09-08 a world on a dead `cifs` share was measured blocking a single call
// for 1m42s -- and the app froze on an ORDINARY SCENE CLICK, showed "not
// responding" after ~6 s, and had to be Force Quit. For a writing application
// that is data loss, not latency.
//
// ⚠️ A TIMER ALONE CANNOT FIX THIS. If the UI thread is inside a blocking
// `read()`, no QTimer fires and no event is processed -- the timer never gets a
// chance to run. Moving the call to a worker thread is what makes a timeout
// EXPRESSIBLE at all; it is the mechanism, not groundwork for it.
//
// ⚠️ WHAT A TIMEOUT HERE DOES AND DOES NOT DO.
//
// ✅ It frees the UI: the writer gets an answer and an app that responds.
// ⚠️ It does NOT cancel the underlying syscall. The worker thread stays blocked
// until the kernel gives up (the `cifs` `soft` mount eventually returns
// EHOSTDOWN). That thread is parked, not leaked -- but a caller MUST NOT assume
// the work stopped, and must not fire the same call again in a tight loop.
//
// ⚠️ The result is DISCARDED if it arrives after the timeout. Reporting a stale
// answer to a writer who has already been told "unreachable" would be worse than
// silence -- see I-0115: a wrong-but-confident status invites destructive
// remedies.
//
// ⚠️ THE CALLABLE RUNS ON A WORKER THREAD. It may touch ONLY the C ABI (which is
// thread-safe per call) and its own locals. It must NEVER touch widgets, models,
// or any Qt object owned by the UI thread. The `onDone`/`onTimeout` handlers run
// back ON the UI thread, which is where UI work belongs.
namespace AsyncCall {

// The default budget for a world/project read.
//
// ⚠️ Chosen from MEASUREMENT, not taste: the healthy path is ~0.09 s and the dead
// share blocks ~102 s, so anything in between separates them cleanly. 5 s is
// ~55x the healthy call -- generous enough that a merely SLOW network share still
// completes (I-0195's case, which must NOT be aborted), and short enough that a
// writer is not left staring at a frozen panel.
//
// ⚠️ Do NOT tighten this toward the healthy figure. A legitimate slow load that
// gets aborted is a worse defect than the freeze it replaced.
inline constexpr int kDefaultTimeoutMs = 5000;

// Run `work` on a worker thread.
//
//   onDone(result)  -- called ON THE UI THREAD if `work` finished in time.
//   onTimeout()     -- called ON THE UI THREAD if it did not.
//
// Exactly ONE of the two runs. `context` owns the lifetime: if it is destroyed
// first, neither fires.
//
// ⚠️ `work` is copied to the worker thread. Capture BY VALUE -- a reference to a
// UI-thread object can dangle, and this is the classic way an async refactor
// introduces a crash that only shows up under load.
template <typename T>
void run(QObject* context,
         std::function<T()> work,
         std::function<void(T)> onDone,
         std::function<void()> onTimeout,
         int timeoutMs = kDefaultTimeoutMs)
{
    auto* watcher = new QFutureWatcher<T>(context);
    auto* timer   = new QTimer(context);
    timer->setSingleShot(true);

    // ⚠️ Shared so BOTH paths can see who won. Without it a slow result that
    // lands just after the timeout would call onDone as well, and the writer
    // would be told two contradictory things about the same world.
    auto settled = std::make_shared<bool>(false);

    QObject::connect(watcher, &QFutureWatcher<T>::finished, context,
                     [watcher, timer, settled, onDone]() {
                         if (*settled) {
                             // Timed out already; the answer is stale. Discard it.
                             watcher->deleteLater();
                             return;
                         }
                         *settled = true;
                         timer->stop();
                         if (onDone) { onDone(watcher->result()); }
                         watcher->deleteLater();
                     });

    QObject::connect(timer, &QTimer::timeout, context,
                     [settled, onTimeout]() {
                         if (*settled) { return; }
                         *settled = true;
                         // ⚠️ The watcher is NOT deleted here -- its future is
                         // still running on a parked worker thread. It deletes
                         // itself in the finished handler above, whenever the
                         // kernel finally releases the call.
                         if (onTimeout) { onTimeout(); }
                     });

    timer->start(timeoutMs);
    watcher->setFuture(QtConcurrent::run(std::move(work)));
}

}  // namespace AsyncCall
