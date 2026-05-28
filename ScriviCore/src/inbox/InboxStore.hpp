#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::inbox {

class InboxStore {
public:
    explicit InboxStore(CoreServices& services);

    Result<ListInboxResult>       list(const ListInboxRequest& request);
    Result<ImportFromInboxResult> importFromInbox(const ImportFromInboxRequest& request);

private:
    CoreServices& services_;

    AbsolutePath droppedFilesDir(const AbsolutePath& projectRoot) const;
};

} // namespace scrivi::inbox
