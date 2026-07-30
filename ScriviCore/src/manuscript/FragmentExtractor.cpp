#include "manuscript/FragmentExtractor.hpp"

#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneReader.hpp"

#include <unordered_map>

namespace scrivi::manuscript {

namespace {

Error invalidArg(std::string message)
{
    return Error{ErrorCode::invalidArgument, std::move(message), "", ""};
}

} // namespace

FragmentExtractor::FragmentExtractor(CoreServices& services)
    : services_(services) {}

Result<Fragment> FragmentExtractor::extract(const AbsolutePath& projectRoot,
                                            const std::vector<FragmentSpan>& spans) const
{
    using R = Result<Fragment>;

    if (spans.empty())
        return R::failure(invalidArg("no spans: a fragment must cover at least one scene"));

    // Resolve the manuscript in reading order so we know each touched scene's chapter
    // membership + title + content path, and can enforce that spans arrive in reading order.
    ManuscriptOrderResolver resolver{services_};
    auto resolvedR = resolver.resolve(projectRoot);
    if (!resolvedR.ok())
        return R::failure(resolvedR.error());
    const auto& ordered = resolvedR.value();

    // sceneID -> manuscript index, so we can (a) look a span's scene up and (b) verify order.
    std::unordered_map<std::string, std::size_t> indexByScene;
    indexByScene.reserve(ordered.size());
    for (std::size_t i = 0; i < ordered.size(); ++i)
        indexByScene.emplace(ordered[i].sceneID.value, i);

    SceneReader reader{services_};

    Fragment fragment;
    fragment.pieces.reserve(spans.size());

    // Track the previous piece's manuscript index + chapter so opensWith is computed relative
    // to the piece before it (design §3: a boundary is "relative to the previous piece").
    bool havePrev = false;
    std::size_t prevIndex = 0;
    std::string prevChapterID;

    for (std::size_t s = 0; s < spans.size(); ++s) {
        const FragmentSpan& span = spans[s];

        if (span.startByte > span.endByte)
            return R::failure(invalidArg("span startByte > endByte for scene " + span.sceneID.value));

        auto it = indexByScene.find(span.sceneID.value);
        if (it == indexByScene.end())
            return R::failure(invalidArg("scene not in manuscript: " + span.sceneID.value));
        const std::size_t index = it->second;

        // Spans must be in strict manuscript reading order (each later than the last), and no
        // scene may appear twice — a selection touches each scene at most once, left to right.
        if (havePrev && index <= prevIndex)
            return R::failure(invalidArg("spans out of manuscript order at scene " + span.sceneID.value));

        const ResolvedScene& scene = ordered[index];

        // Read this scene's body (raw markdown — no dividers/headings; those are UI-only) and
        // slice the requested byte span. std::string is byte-addressed, matching the scene-local
        // UTF-8 byte-offset convention.
        auto bodyR = reader.readContent(projectRoot, scene.contentPath);
        if (!bodyR.ok())
            return R::failure(bodyR.error());
        const std::string& body = bodyR.value();

        if (span.endByte > body.size())
            return R::failure(invalidArg("span endByte past end of scene body for scene "
                                         + span.sceneID.value));

        FragmentPiece piece;
        piece.text       = body.substr(span.startByte, span.endByte - span.startByte);
        piece.sceneTitle = scene.title;   // capture the scene's title so paste can restore it (T-0357)

        // opensWith — relative to the previous piece (design §3):
        //   first piece      -> None  (the selection began mid-/at-scene; leading edge)
        //   new chapter       -> Chapter (+ chapterTitle)
        //   same chapter      -> Scene
        if (!havePrev) {
            piece.opensWith = OpensWith::None;
        } else if (scene.chapterID.value != prevChapterID) {
            piece.opensWith    = OpensWith::Chapter;
            piece.chapterTitle = scene.chapterTitle;
        } else {
            piece.opensWith = OpensWith::Scene;
        }

        fragment.pieces.push_back(std::move(piece));

        havePrev      = true;
        prevIndex     = index;
        prevChapterID = scene.chapterID.value;
    }

    // partial (design §3): the leading piece is a head partial if the selection began past the
    // start of its scene; the trailing piece is a tail partial if it ended before the end of its
    // scene. Interior pieces are always whole scenes. For a single-piece fragment both edge
    // conditions apply to the one piece; head is recorded (the editor never calls extract for a
    // single-scene selection — AC5 — but the model stays total).
    {
        const FragmentSpan& first = spans.front();
        if (first.startByte > 0)
            fragment.pieces.front().partial = Partial::Head;

        const FragmentSpan& last = spans.back();
        // Re-read the last scene's length to decide the tail. (Cheap: one extra read only when
        // the last piece differs from the first; for a single-piece fragment we reuse below.)
        auto lastIt = indexByScene.find(last.sceneID.value);
        const ResolvedScene& lastScene = ordered[lastIt->second];
        auto lastBodyR = reader.readContent(projectRoot, lastScene.contentPath);
        if (!lastBodyR.ok())
            return R::failure(lastBodyR.error());
        if (last.endByte < lastBodyR.value().size()) {
            // Only overwrite to Tail when this isn't already the (single) head piece, so a
            // one-piece fragment keeps Head; a multi-piece fragment marks its distinct last piece.
            if (fragment.pieces.size() > 1)
                fragment.pieces.back().partial = Partial::Tail;
            else if (fragment.pieces.front().partial == Partial::None)
                fragment.pieces.front().partial = Partial::Tail;
        }
    }

    // plainText — pieces joined by a blank-line seam, for the system pasteboard (T2=A).
    for (std::size_t i = 0; i < fragment.pieces.size(); ++i) {
        if (i > 0)
            fragment.plainText += "\n\n";
        fragment.plainText += fragment.pieces[i].text;
    }

    return R::success(std::move(fragment));
}

} // namespace scrivi::manuscript
