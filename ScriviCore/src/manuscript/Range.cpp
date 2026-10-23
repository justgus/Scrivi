#include "manuscript/Range.hpp"

#include "util/Json.hpp"

#include <iterator>

namespace scrivi::manuscript {

namespace {

constexpr std::string_view kSchema = "scrivi.range.v1";

Error invalid(std::string message) { return Error{.code = ErrorCode::invalidArgument, .message = std::move(message)}; }

}   // namespace

util::JsonDoc Range::toJson() const
{
    util::JsonDoc doc;
    doc.setString("schema", kSchema);
    for (const RangeEntry& e : entries_) {
        util::JsonDoc item;
        item.setString("sceneID", e.sceneID.value);
        item.setString("chapterID", e.chapterID.value);
        item.setString("chapterTitle", e.chapterTitle);
        item.setString("sceneTitle", e.sceneTitle);
        item.setInt64("start", static_cast<int64_t>(e.start));
        item.setInt64("length", static_cast<int64_t>(e.length));
        item.setInt64("sceneLength", static_cast<int64_t>(e.sceneLength));
        item.setString("text", e.text);
        if (!e.sceneMeta.empty()) { item.setString("sceneMeta", e.sceneMeta); }
        if (!e.chapterMeta.empty()) { item.setString("chapterMeta", e.chapterMeta); }
        doc.appendToArray("scenes", std::move(item));
    }
    if (join_) {
        util::JsonDoc j;
        j.setString("sceneID", join_->sceneID.value);
        j.setInt64("byte", static_cast<int64_t>(join_->byte));
        j.setString("balanced", join_->balanced);
        j.setString("plain", join_->plain);
        doc.setSubDoc("join", std::move(j));
    }
    return doc;
}

std::string Range::toJsonString() const { return toJson().dump(-1); }

Result<Range> Range::fromJson(const util::JsonDoc& doc)
{
    using R = Result<Range>;
    if (!doc.isObject() || doc.getString("schema") != kSchema)
        return R::failure(invalid("not a scrivi.range.v1 document"));
    Range out;
    const std::size_t n = doc.arraySize("scenes");
    if (n == 0)
        return R::failure(invalid("a range covers at least one scene"));
    for (std::size_t i = 0; i < n; ++i) {
        const util::JsonDoc item = doc.arrayItem("scenes", i);
        RangeEntry e;
        e.sceneID.value   = item.getString("sceneID");
        e.chapterID.value = item.getString("chapterID");
        e.chapterTitle    = item.getString("chapterTitle");
        e.sceneTitle      = item.getString("sceneTitle");
        const int64_t start = item.getInt64("start"), length = item.getInt64("length"), whole = item.getInt64("sceneLength");
        e.text            = item.getString("text");
        e.sceneMeta       = item.getString("sceneMeta");
        e.chapterMeta     = item.getString("chapterMeta");
        if (start < 0 || length < 0 || whole < 0 || start + length > whole)
            return R::failure(invalid("range entry " + std::to_string(i) + " has offsets outside its scene"));
        if (static_cast<std::size_t>(length) != e.text.size())
            return R::failure(invalid("range entry " + std::to_string(i) + "'s length is not its text's"));
        // Contiguity (R2): only the FIRST entry may start mid-scene, only the LAST may end mid-scene.
        if (i > 0 && start != 0)
            return R::failure(invalid("range entry " + std::to_string(i) + " starts mid-scene (only the first may)"));
        if (i + 1 < n && start + length != whole)
            return R::failure(invalid("range entry " + std::to_string(i) + " ends mid-scene (only the last may)"));
        e.start = static_cast<std::size_t>(start);
        e.length = static_cast<std::size_t>(length);
        e.sceneLength = static_cast<std::size_t>(whole);
        out.append(std::move(e));
    }
    if (doc.contains("join")) {
        const util::JsonDoc j = doc.getSubDoc("join");
        const int64_t byte = j.getInt64("byte", -1);
        if (!j.isObject() || j.getString("sceneID").empty() || byte < 0)
            return R::failure(invalid("the range's join names no scene or no byte"));
        out.setJoin(RangeJoin{.sceneID = SceneID{j.getString("sceneID")}, .byte = static_cast<std::size_t>(byte),
                              .balanced = j.getString("balanced"), .plain = j.getString("plain")});
    }
    return R::success(std::move(out));
}

Result<Range> Range::fromJsonString(std::string_view json)
{
    auto parsed = util::parseJson(json);
    if (!parsed.ok())
        return Result<Range>::failure(parsed.error());
    return fromJson(parsed.value());
}

Fragment Range::toFragment() const
{
    Fragment f;
    f.pieces.reserve(entries_.size());
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        FragmentPiece p;
        p.text = it->text;
        p.sceneTitle = it->sceneTitle;
        if (it == entries_.begin()) {
            p.opensWith = OpensWith::None;
        } else if (opensChapter(std::prev(it), it)) {
            p.opensWith = OpensWith::Chapter;
            p.chapterTitle = it->chapterTitle;
        } else {
            p.opensWith = OpensWith::Scene;
        }
        f.pieces.push_back(std::move(p));
    }
    // partial (fragment v1 §3): the first piece is a head partial if it starts past its scene's start; the last a tail
    // partial if it ends before its scene's end. A one-piece fragment keeps Head (v1's rule).
    if (!entries_.empty()) {
        if (entries_.front().start > 0) {
            f.pieces.front().partial = Partial::Head;
        }
        const RangeEntry& last = entries_.back();
        if (last.start + last.length < last.sceneLength && f.pieces.size() > 1) {
            f.pieces.back().partial = Partial::Tail;
        }
    }
    for (std::size_t i = 0; i < f.pieces.size(); ++i) {
        f.plainText += (i ? "\n\n" : "") + f.pieces[i].text;
    }
    return f;
}

Range Range::fromFragment(const Fragment& fragment)
{
    Range out;
    int chapter = 0;
    std::string chapterID = "~fragment-chapter-0";
    for (std::size_t i = 0; i < fragment.pieces.size(); ++i) {
        const FragmentPiece& p = fragment.pieces[i];
        RangeEntry e;
        e.sceneID.value = "~fragment-scene-" + std::to_string(i);   // a fragment carries text, not identity
        if (p.opensWith == OpensWith::Chapter) {
            chapterID = "~fragment-chapter-" + std::to_string(++chapter);
            e.chapterTitle = p.chapterTitle;
        }
        e.chapterID.value = chapterID;
        e.sceneTitle = p.sceneTitle;
        e.text = p.text;
        e.length = p.text.size();
        e.sceneLength = p.text.size();
        out.append(std::move(e));
    }
    return out;
}

}   // namespace scrivi::manuscript
