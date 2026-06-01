#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::inbox {

class InboxStore {
public:
    explicit InboxStore(CoreServices& services);

    [[nodiscard]] Result<ListInboxResult> list(const ListInboxRequest& request) const;
    Result<ImportFromInboxResult> importFromInbox(const ImportFromInboxRequest& request);

private:
    CoreServices& services_;

    [[nodiscard]] static AbsolutePath droppedFilesDir(const AbsolutePath& projectRoot);
};

} // namespace scrivi::inbox
