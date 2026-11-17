#include "manuscript/ManuscriptScrivener.hpp"

#include "manuscript/ManuscriptDocx.hpp"   // inlineMarkdown
#include "manuscript/ManuscriptRtf.hpp"
#include "schemas/SceneMetaJson.hpp"

#include "util/PathUtils.hpp"

#include "miniz.h"
#include "pugixml.hpp"

#include <algorithm>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <vector>

namespace scrivi::manuscript {

namespace {

// Any one file the import reads (a document's RTF, the project file); a picture-heavy document is megabytes, not this.
constexpr std::uint64_t kMaxFile = 64u << 20;

Error tooLarge() {
    return Error{ErrorCode::invalidArgument, "a file in the project is too large to be a manuscript", {}, "tooLarge"};
}

Error notScrivener() {
    return Error{ErrorCode::invalidArgument,
                 "This is not a Scrivener project Scrivi can read. Open it in Scrivener 3, and import that.", {}, "notScrivener"};
}

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\n' || s.front() == '\r')) { s.remove_prefix(1); }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\n' || s.back() == '\r')) { s.remove_suffix(1); }
    return s;
}

std::string shortened(std::string_view line) {
    line = trim(line);
    if (line.size() <= 80) { return std::string(line); }
    std::size_t n = 80;
    while (n > 0 && (static_cast<unsigned char>(line[n]) & 0xC0) == 0x80) { --n; }
    return std::string(line.substr(0, n)) + "…";
}

// A binder item's ID becomes part of a path: only the characters a UUID or a number has (never `/` or `..`).
bool safeID(std::string_view id) {
    return !id.empty() && id.size() <= 64 && std::all_of(id.begin(), id.end(), [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '-';
    });
}

struct Notes {
    std::map<std::string, ImportNote> byKind;
    std::vector<std::string> order;
    void add(const std::string& kind, std::string_view example, std::size_t count = 1) {
        auto [it, inserted] = byKind.try_emplace(kind, ImportNote{kind, 0, shortened(example)});
        if (inserted) { order.push_back(kind); }
        it->second.count += count;
    }
};

struct Scene {
    std::string title, text, seed;
};

struct Chapter {
    pugi::xml_node owner;   // the folder whose run this is; the Draft itself for documents straight in it
    std::string title;
    std::vector<Scene> scenes;
};

struct Walker {
    const ScrivenerFileReader& read;
    bool dataLayout = true;   // Scrivener 3: Files/Data/<UUID>/…; else Files/Docs/<ID>…
    pugi::xml_node draft;
    std::map<std::string, std::string> statuses, keywords;
    std::vector<Chapter> chapters;
    Notes notes;
    // Q4 formatting: the faces and flags of every paragraph, decided once the project's own face is known.
    struct Para { std::set<std::string> faces; bool formatted = false; std::string example; };
    std::vector<Para> paras;
    std::map<std::string, std::size_t> faceChars;
    std::optional<Error> failed;

    // The path of one of a document's files: "content.rtf", "notes.rtf", "synopsis.txt", "content.comments".
    std::optional<std::string> path(const pugi::xml_node& item, std::string_view file) const {
        const std::string_view id = dataLayout ? item.attribute("UUID").as_string() : item.attribute("ID").as_string();
        if (!safeID(id)) { return std::nullopt; }
        if (file == "snapshots") { return "Snapshots/" + std::string(id) + ".snapshots/index.xml"; }   // both layouts
        if (dataLayout) { return "Files/Data/" + std::string(id) + "/" + std::string(file); }
        const std::string base = "Files/Docs/" + std::string(id);
        if (file == "content.rtf") { return base + ".rtf"; }
        if (file == "notes.rtf") { return base + "_notes.rtf"; }
        if (file == "synopsis.txt") { return base + "_synopsis.txt"; }
        return base + ".comments";
    }

    std::optional<std::string> file(const pugi::xml_node& item, std::string_view name) {
        if (failed) { return std::nullopt; }
        const auto p = path(item, name);
        if (!p) { return std::nullopt; }
        auto got = read(*p);
        if (!got.ok()) { failed = got.error(); return std::nullopt; }
        return got.value();
    }

    static std::string titleOf(const pugi::xml_node& item) { return std::string(trim(item.child_value("Title"))); }

    Chapter& runFor(const pugi::xml_node& owner) {
        if (chapters.empty() || chapters.back().owner != owner) {
            chapters.push_back({owner, owner == draft ? std::string() : titleOf(owner), {}});
        }
        return chapters.back();
    }

    void document(const pugi::xml_node& item, const pugi::xml_node& owner) {
        const std::string title = titleOf(item);
        const std::string name = title.empty() ? std::string("(untitled)") : title;
        const auto meta = item.child("MetaData");
        if (std::string_view(meta.child_value("IncludeInCompile")) != "Yes") { notes.add("notCompiled", name); }
        if (const std::string_view label = meta.child_value("LabelID"); !label.empty() && label != "-1") { notes.add("label", name); }
        if (meta.child("CustomMetaData").child("MetaDataItem")) { notes.add("customMetadata", name); }

        // Q4: the seed a paste gives the new scene.
        schemas::SceneMetaData seed;
        seed.title  = title;
        seed.status = "draft";
        if (const std::string_view s = meta.child_value("StatusID"); !s.empty() && s != "-1") {
            const std::string status = statuses.contains(std::string(s)) ? statuses[std::string(s)] : std::string();
            if (status == "First Draft") { seed.status = "draft"; }
            else if (status == "Revised Draft") { seed.status = "revised"; }
            else if (status == "Final Draft" || status == "Done") { seed.status = "final"; }
            else { notes.add("status", (status.empty() ? std::string("a status") : "“" + status + "”") + " on " + name); }
        }
        for (auto k : item.child("Keywords").children("KeywordID")) {
            if (auto it = keywords.find(std::string(trim(k.child_value()))); it != keywords.end() &&
                std::find(seed.tags.begin(), seed.tags.end(), it->second) == seed.tags.end()) {
                seed.tags.push_back(it->second);
            }
        }
        if (auto synopsis = file(item, "synopsis.txt")) { seed.outline = std::string(trim(*synopsis)); }
        if (auto n = file(item, "notes.rtf")) {
            if (!readRtf(*n).paragraphs.empty()) { notes.add("documentNotes", name); }
        }
        if (file(item, "snapshots")) { notes.add("snapshots", name); }
        if (auto c = file(item, "content.comments")) {
            pugi::xml_document doc;
            if (doc.load_buffer(c->data(), c->size())) {
                for (auto k : doc.document_element().children("Comment")) {
                    notes.add(std::string_view(k.attribute("Footnote").as_string()) == "Yes" ? "footnote" : "comment", name);
                }
            }
        }

        // The text: one scene, or more where a paragraph of only `* * *`, `***` or `#` breaks it.
        std::vector<Scene> scenes{Scene{title, {}, {}}};
        if (auto content = file(item, "content.rtf")) {
            const RtfText rtf = readRtf(*content);
            if (!rtf.rtf && !trim(*content).empty()) { notes.add("notRtf", name); }
            if (rtf.pictures > 0) { notes.add("picture", name, rtf.pictures); }
            if (rtf.tables > 0) { notes.add("docxTable", name, rtf.tables); }   // the text kept, cell by cell (S6b's wording)
            if (rtf.footnotes > 0) { notes.add("footnote", name, rtf.footnotes); }
            if (rtf.annotations > 0) { notes.add("comment", name, rtf.annotations); }
            for (const auto& link : rtf.links) {
                if (link.starts_with("scrivcmt:")) { continue; }   // a comment's anchor: counted from content.comments
                notes.add(link.starts_with("scrivlnk:") ? "documentLink" : "link", name);
            }
            for (const auto& [face, n] : rtf.faceChars) { faceChars[face] += n; }
            for (const auto& p : rtf.paragraphs) {
                std::string plain;
                for (const auto& r : p.runs) { plain += r.text; }
                const std::string_view t = trim(plain);
                if (t == "* * *" || t == "***" || t == "#") { scenes.push_back({}); continue; }
                const std::string md = inlineMarkdown(p.runs);
                if (md.empty()) { continue; }
                auto& s = scenes.back().text;
                s += (s.empty() ? "" : "\n\n") + md;
                paras.push_back({p.faces, p.formatted, std::string(t)});
            }
        }
        Chapter& ch = runFor(owner);
        for (std::size_t i = 0; i < scenes.size(); ++i) {
            if (i == 0) {
                const bool plainSeed = seed.outline.empty() && seed.tags.empty() && seed.status == "draft";
                if (!plainSeed) { scenes[0].seed = schemas::serializeSceneMeta(seed); }
            }
            ch.scenes.push_back(std::move(scenes[i]));
        }
    }

    // `owner`: the folder whose chapter this level's documents belong to.
    void walk(const pugi::xml_node& children, const pugi::xml_node& owner) {
        for (auto item : children.children("BinderItem")) {
            if (failed) { return; }
            const std::string_view type = item.attribute("Type").as_string();
            if (type == "Folder") {
                const auto kids = item.child("Children");
                bool hasText = false, hasFolder = false, any = false;
                for (auto k : kids.children("BinderItem")) {
                    any = true;
                    (std::string_view(k.attribute("Type").as_string()) == "Folder" ? hasFolder : hasText) = true;
                }
                if (!any) { runFor(item); continue; }   // an empty folder is an empty chapter
                if (hasFolder && !hasText) { notes.add("parts", titleOf(item)); }
                walk(kids, item);
                continue;
            }
            if (type == "Text") { document(item, owner); }
            else { notes.add("media", titleOf(item)); }   // a picture, PDF or other file in the Draft
            if (auto kids = item.child("Children")) { walk(kids, owner); }   // Q3a: a text's children are scenes too
        }
    }

    // Q2: what is outside the Draft, counted (folders themselves are not items).
    void outside(const pugi::xml_node& children, std::size_t& count, std::string& first) {
        for (auto item : children.children("BinderItem")) {
            const std::string_view type = item.attribute("Type").as_string();
            if (type == "DraftFolder") { continue; }
            if (type != "Folder" && type != "ResearchFolder" && type != "TrashFolder") {
                ++count;
                if (first.empty()) { first = titleOf(item).empty() ? "(untitled)" : titleOf(item); }
            }
            outside(item.child("Children"), count, first);
        }
    }

    void formattingNote() {
        std::string common;
        std::size_t most = 0;
        for (const auto& [face, n] : faceChars) { if (n > most) { most = n; common = face; } }
        for (const auto& p : paras) {
            const bool otherFace = std::any_of(p.faces.begin(), p.faces.end(), [&](const std::string& f) { return f != common; });
            if (p.formatted || otherFace) { notes.add("formatting", p.example); }
        }
    }
};

} // namespace

Result<ImportedManuscript> importScrivener(std::string_view scrivx, const ScrivenerFileReader& read) {
    using R = Result<ImportedManuscript>;
    pugi::xml_document doc;
    if (!doc.load_buffer(scrivx.data(), scrivx.size())) { return R::failure(notScrivener()); }   // no DTD is read
    const auto root = doc.child("ScrivenerProject");
    const std::string_view version = root.attribute("Version").as_string();
    // Q1: 2.x is Scrivener 3's layout, 1.x Mac 2 / Windows 1; anything else refused.
    if (!root || !(version.starts_with("2.") || version.starts_with("1."))) { return R::failure(notScrivener()); }
    const auto binder = root.child("Binder");
    pugi::xml_node draft;
    for (auto item : binder.children("BinderItem")) {
        if (std::string_view(item.attribute("Type").as_string()) == "DraftFolder") { draft = item; break; }
    }
    if (!draft) { return R::failure(notScrivener()); }

    Walker w{read};
    w.dataLayout = version.starts_with("2.");
    w.draft = draft;
    for (auto s : root.child("StatusSettings").child("StatusItems").children("Status")) {
        w.statuses[s.attribute("ID").as_string()] = std::string(trim(s.child_value()));
    }
    for (auto k : root.child("Keywords").children("Keyword")) {
        w.keywords[k.attribute("ID").as_string()] = std::string(trim(k.child_value("Title")));
    }
    w.walk(draft.child("Children"), draft);
    if (w.failed) { return R::failure(*w.failed); }
    std::size_t outsideCount = 0;
    std::string outsideFirst;
    w.outside(binder, outsideCount, outsideFirst);
    if (outsideCount > 0) { w.notes.add("outsideDraft", outsideFirst, outsideCount); }
    w.formattingNote();

    ImportedManuscript out;
    out.encoding = "utf-8";
    for (std::size_t ci = 0; ci < w.chapters.size(); ++ci) {
        auto& c = w.chapters[ci];
        if (c.scenes.empty()) { c.scenes.push_back({}); }   // an empty folder keeps its chapter
        ++out.chapterCount;
        for (auto& s : c.scenes) {
            RangeEntry e;
            e.chapterID = ChapterID{"~import-" + std::to_string(ci)};
            e.chapterTitle = c.title;
            e.sceneTitle = std::move(s.title);
            e.sceneMeta = std::move(s.seed);
            e.length = e.sceneLength = s.text.size();
            e.text = std::move(s.text);
            out.range.append(std::move(e));
        }
    }
    out.opensChapter = !w.chapters.empty() && w.chapters.front().owner != draft;
    for (const auto& k : w.notes.order) { out.report.push_back(w.notes.byKind[k]); }
    return R::success(std::move(out));
}

Result<ImportedManuscript> importScrivenerFolder(FileSystem& fs, const std::string& folder) {
    using R = Result<ImportedManuscript>;
    auto listed = fs.listDirectory(folder);
    if (!listed.ok()) { return R::failure(listed.error()); }
    std::string scrivx;
    for (const auto& p : listed.value()) {
        if (p.ends_with(".scrivx")) {
            if (!scrivx.empty()) { return R::failure(notScrivener()); }   // two project files: not one project
            scrivx = p;
        }
    }
    if (scrivx.empty()) { return R::failure(notScrivener()); }
    const ScrivenerFileReader read = [&](std::string_view rel) -> Result<std::optional<std::string>> {
        using F = Result<std::optional<std::string>>;
        const std::string path = rel.empty() ? scrivx : util::join(folder, rel);
        std::error_code ec;
        if (!std::filesystem::is_regular_file(path, ec)) { return F::success(std::nullopt); }
        if (std::filesystem::file_size(path, ec) > kMaxFile) { return F::failure(tooLarge()); }
        auto text = fs.readTextFile(path);
        if (!text.ok()) { return F::failure(text.error()); }
        return F::success(std::move(text.value()));
    };
    auto project = read("");
    if (!project.ok()) { return R::failure(project.error()); }
    if (!project.value()) { return R::failure(notScrivener()); }
    return importScrivener(*project.value(), read);
}

Result<ImportedManuscript> importScrivenerZip(const std::string& zipPath) {
    using R = Result<ImportedManuscript>;
    mz_zip_archive z{};
    if (!mz_zip_reader_init_file(&z, zipPath.c_str(), 0)) { return R::failure(notScrivener()); }
    struct Closer { mz_zip_archive* z; ~Closer() { mz_zip_reader_end(z); } } closer{&z};
    // The project file is `<Name>.scriv/<Name>.scrivx`, one folder deep (so macOS's `__MACOSX/…` copies are not it).
    std::string prefix;
    int project = -1;
    const auto count = mz_zip_reader_get_num_files(&z);
    for (mz_uint i = 0; i < count; ++i) {
        char name[1024];
        if (mz_zip_reader_get_filename(&z, i, name, sizeof name) == 0) { continue; }
        const std::string_view n(name);
        const auto slash = n.find('/');
        if (!n.ends_with(".scrivx") || slash == std::string_view::npos ||
            n.find('/', slash + 1) != std::string_view::npos || !n.substr(0, slash).ends_with(".scriv")) {
            continue;
        }
        if (project >= 0) { return R::failure(notScrivener()); }   // two projects in one backup
        project = static_cast<int>(i);
        prefix = std::string(n.substr(0, slash + 1));
    }
    if (project < 0) { return R::failure(notScrivener()); }
    auto extract = [&](int index) -> Result<std::string> {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&z, static_cast<mz_uint>(index), &st)) { return Result<std::string>::failure(notScrivener()); }
        if (st.m_uncomp_size > kMaxFile) { return Result<std::string>::failure(tooLarge()); }
        std::size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&z, static_cast<mz_uint>(index), &size, 0);
        if (data == nullptr) { return Result<std::string>::failure(notScrivener()); }
        std::string out(static_cast<const char*>(data), size);
        mz_free(data);
        return Result<std::string>::success(std::move(out));
    };
    const ScrivenerFileReader read = [&](std::string_view rel) -> Result<std::optional<std::string>> {
        using F = Result<std::optional<std::string>>;
        const int index = mz_zip_reader_locate_file(&z, (prefix + std::string(rel)).c_str(), nullptr, 0);
        if (index < 0) { return F::success(std::nullopt); }
        auto got = extract(index);
        if (!got.ok()) { return F::failure(got.error()); }
        return F::success(std::move(got.value()));
    };
    auto scrivx = extract(project);
    if (!scrivx.ok()) { return R::failure(scrivx.error()); }
    return importScrivener(scrivx.value(), read);
}

} // namespace scrivi::manuscript
