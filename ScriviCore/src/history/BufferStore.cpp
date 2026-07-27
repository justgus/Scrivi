#include "history/BufferStore.hpp"

#include "util/Json.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

namespace scrivi::history {

namespace {
constexpr const char* kBuffersSchema = "scrivi.buffers.v1";
constexpr const char* kBuffersFile   = "buffers.json";
} // namespace

BufferStore::BufferStore(AbsolutePath historyDir, FileSystem* fs, Clock* clock)
    : historyDir_(std::move(historyDir)), fs_(fs), clock_(clock) {}

bool BufferStore::isValidBufferID(const std::string& bufferID) {
    // v1: single digit "1".."9".
    return bufferID.size() == 1 && bufferID[0] >= '1' && bufferID[0] <= '9';
}

AbsolutePath BufferStore::bufferPath() const {
    return util::join(historyDir_, kBuffersFile);
}

Result<std::vector<BufferSlot>> BufferStore::readAll() {
    std::vector<BufferSlot> slots;
    if (fs_ == nullptr) {
        return Result<std::vector<BufferSlot>>::success(std::move(slots));
    }
    auto existsR = fs_->exists(bufferPath());
    if (!existsR.ok()) {
        return Result<std::vector<BufferSlot>>::failure(existsR.error());
    }
    if (!existsR.value()) {
        return Result<std::vector<BufferSlot>>::success(std::move(slots));  // fresh
    }
    auto textR = fs_->readTextFile(bufferPath());
    if (!textR.ok()) {
        return Result<std::vector<BufferSlot>>::failure(textR.error());
    }
    auto parseR = util::parseJson(textR.value());
    if (!parseR.ok()) {
        // A corrupt/unparseable buffers file is treated as empty rather than an error:
        // the slots are a convenience, not manuscript content, and must never block the
        // editor. The next load() overwrites it cleanly.
        return Result<std::vector<BufferSlot>>::success(std::move(slots));
    }
    const auto& doc = parseR.value();
    const std::size_t n = doc.arraySize("buffers");
    for (std::size_t i = 0; i < n; ++i) {
        const auto item = doc.arrayItem("buffers", i);
        BufferSlot slot;
        slot.bufferID  = item.getString("bufferID");
        slot.text      = item.getString("text");
        slot.updatedAt = item.getString("updatedAt");
        if (isValidBufferID(slot.bufferID)) {   // ignore out-of-range/garbage entries
            slots.push_back(std::move(slot));
        }
    }
    std::sort(slots.begin(), slots.end(),
              [](const BufferSlot& a, const BufferSlot& b) { return a.bufferID < b.bufferID; });
    return Result<std::vector<BufferSlot>>::success(std::move(slots));
}

Result<void> BufferStore::writeAll(std::vector<BufferSlot> slots) {
    if (fs_ == nullptr) {
        return Result<void>::failure(
            Error{ErrorCode::internalError, "no filesystem service"});
    }
    // history/ may not exist yet if the writer loads a buffer before any undo history
    // has been recorded — create it (the same dir HistoryStore uses).
    auto mkR = fs_->createDirectories(historyDir_);
    if (!mkR.ok()) {
        return Result<void>::failure(mkR.error());
    }
    std::sort(slots.begin(), slots.end(),
              [](const BufferSlot& a, const BufferSlot& b) { return a.bufferID < b.bufferID; });

    util::JsonDoc doc;
    doc.setString("schema", kBuffersSchema);
    for (const auto& slot : slots) {
        util::JsonDoc item;
        item.setString("bufferID",  slot.bufferID);
        // label is reserved (null in v1) — omitted rather than written as an empty string,
        // so the on-disk shape matches Appendix A.3 without a null-writing helper.
        item.setString("text",      slot.text);
        item.setString("updatedAt", slot.updatedAt);
        doc.appendToArray("buffers", std::move(item));
    }
    return fs_->atomicWriteTextFile(bufferPath(), doc.dump(2));
}

Result<std::string> BufferStore::load(const std::string& bufferID, const std::string& text,
                                      const std::string& nowTimestamp) {
    if (!isValidBufferID(bufferID)) {
        return Result<std::string>::failure(
            Error{ErrorCode::invalidArgument, "bufferID must be \"1\"-\"9\""});
    }
    auto slotsR = readAll();
    if (!slotsR.ok()) {
        return Result<std::string>::failure(slotsR.error());
    }
    auto slots = std::move(slotsR.value());
    const std::string stamp = nowTimestamp.empty()
        ? (clock_ ? clock_->nowUTC() : std::string{})
        : nowTimestamp;

    // Create-or-replace the slot.
    auto it = std::find_if(slots.begin(), slots.end(),
                           [&](const BufferSlot& s) { return s.bufferID == bufferID; });
    if (it != slots.end()) {
        it->text = text;
        it->updatedAt = stamp;
    } else {
        slots.push_back(BufferSlot{bufferID, text, stamp});
    }

    auto writeR = writeAll(std::move(slots));
    if (!writeR.ok()) {
        return Result<std::string>::failure(writeR.error());
    }
    return Result<std::string>::success(stamp);
}

Result<std::optional<BufferSlot>> BufferStore::get(const std::string& bufferID) {
    if (!isValidBufferID(bufferID)) {
        return Result<std::optional<BufferSlot>>::failure(
            Error{ErrorCode::invalidArgument, "bufferID must be \"1\"-\"9\""});
    }
    auto slotsR = readAll();
    if (!slotsR.ok()) {
        return Result<std::optional<BufferSlot>>::failure(slotsR.error());
    }
    for (auto& slot : slotsR.value()) {
        if (slot.bufferID == bufferID) {
            return Result<std::optional<BufferSlot>>::success(std::move(slot));
        }
    }
    return Result<std::optional<BufferSlot>>::success(std::nullopt);  // empty slot
}

Result<std::vector<BufferSlot>> BufferStore::list() {
    return readAll();
}

Result<bool> BufferStore::clear(const std::string& bufferID) {
    if (!isValidBufferID(bufferID)) {
        return Result<bool>::failure(
            Error{ErrorCode::invalidArgument, "bufferID must be \"1\"-\"9\""});
    }
    auto slotsR = readAll();
    if (!slotsR.ok()) {
        return Result<bool>::failure(slotsR.error());
    }
    auto slots = std::move(slotsR.value());
    const auto before = slots.size();
    slots.erase(std::remove_if(slots.begin(), slots.end(),
                               [&](const BufferSlot& s) { return s.bufferID == bufferID; }),
                slots.end());
    const bool removed = slots.size() != before;
    if (!removed) {
        return Result<bool>::success(false);  // already empty — no write needed
    }
    auto writeR = writeAll(std::move(slots));
    if (!writeR.ok()) {
        return Result<bool>::failure(writeR.error());
    }
    return Result<bool>::success(true);
}

} // namespace scrivi::history
