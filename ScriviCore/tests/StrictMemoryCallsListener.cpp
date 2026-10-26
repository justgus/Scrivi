// EP-052 (SP-194, T-0623): in the test run, a `scrivi_*` endpoint marked MEMORY_ that makes a file operation aborts, so
// every test that reaches a misclassified endpoint fails, not only the tests written for the lock.
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "public_api/ProjectLock.hpp"

namespace {

class StrictMemoryCalls : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;
    void testRunStarting(Catch::TestRunInfo const&) override { scrivi::abi::setMemoryCallViolationsFatal(true); }
};

}  // namespace

CATCH_REGISTER_LISTENER(StrictMemoryCalls)
