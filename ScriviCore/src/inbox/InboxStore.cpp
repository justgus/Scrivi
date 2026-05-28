#include "inbox/InboxStore.hpp"

#include "assets/AssetStore.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::inbox {

InboxStore::InboxStore(CoreServices& services)
    : services_(services) {}

AbsolutePath InboxStore::droppedFilesDir(const AbsolutePath& projectRoot) const {
    return util::join(util::join(projectRoot, "inbox"), "dropped-files");
}

// ---------------------------------------------------------------------------
// list
// ---------------------------------------------------------------------------

Result<ListInboxResult> InboxStore::list(const ListInboxRequest& request)
{
    auto& fs = *services_.fileSystem;
    auto dir = droppedFilesDir(request.projectRootPath);

    auto existsR = fs.exists(dir);
    if (!existsR.ok() || !existsR.value())
        return Result<ListInboxResult>::success(ListInboxResult{});

    auto listR = fs.listDirectory(dir);
    if (!listR.ok()) return Result<ListInboxResult>::failure(listR.error());

    ListInboxResult result;
    for (const auto& entry : listR.value()) {
        InboxEntry e;
        e.filename     = util::filename(entry);
        e.absolutePath = entry;
        result.entries.push_back(std::move(e));
    }
    return Result<ListInboxResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// importFromInbox
// ---------------------------------------------------------------------------

Result<ImportFromInboxResult> InboxStore::importFromInbox(
    const ImportFromInboxRequest& request)
{
    auto& fs = *services_.fileSystem;
    auto inboxPath = util::join(droppedFilesDir(request.projectRootPath),
                                request.filename);

    switch (request.action) {
        case InboxAction::ignore: {
            ImportFromInboxResult r;
            r.actionTaken = "ignored";
            return Result<ImportFromInboxResult>::success(std::move(r));
        }
        case InboxAction::deleteFile: {
            if (auto dr = fs.removeFile(inboxPath); !dr.ok())
                return Result<ImportFromInboxResult>::failure(dr.error());
            ImportFromInboxResult r;
            r.actionTaken = "deleted";
            return Result<ImportFromInboxResult>::success(std::move(r));
        }
        case InboxAction::importAsAsset: {
            ImportAssetRequest assetReq;
            assetReq.projectRootPath = request.projectRootPath;
            assetReq.sourcePath      = inboxPath;
            assetReq.category        = request.assetCategory;
            assetReq.title           = request.filename;
            assetReq.author          = request.author;

            assets::AssetStore assetStore{services_};
            auto importR = assetStore.import(assetReq);
            if (!importR.ok())
                return Result<ImportFromInboxResult>::failure(importR.error());

            // Remove the original inbox file after successful import.
            (void)fs.removeFile(inboxPath);

            ImportFromInboxResult r;
            r.actionTaken = "importAsAsset";
            r.resultPath  = importR.value().assetPath;
            r.assetID     = importR.value().assetID;
            return Result<ImportFromInboxResult>::success(std::move(r));
        }
    }
    return Result<ImportFromInboxResult>::failure(
        {ErrorCode::invalidArgument, "unknown InboxAction"});
}

} // namespace scrivi::inbox
