import Foundation

// Per-project preferences that TRAVEL with the project (EP-047 AC1–AC3, SP-167, [I-0278]).
//
// ✅ Subtitle and "Show chapter titles" live in the package's `project-settings.json` (through ScriviCore —
// `scrivi_get/put_project_settings`); the TITLE is `project.json`'s own (`scrivi_set_project_title`).
// ⛔ They used to live in this Mac's `UserDefaults` (`scrivi.project.<id>.preferences`), so they were lost on
// another Mac — [I-0278]. That key is migrated ONCE and then deleted (Q2, below).
//
// ⚠️ The settings document is shared with Linux and with later builds, so it is kept WHOLE: a save merges
// this class's keys into the document it read and writes all of it back ([I-0215]: never drop a key you
// did not write). Keys: `subtitle` (string), `showChapterTitles` (bool) — `docs/Scrivi_Project_Package_Structure_v0_1.md`.
//
// Created by ProjectSession when a project opens; cleared when it closes.
@Observable @MainActor final class ProjectPreferences {

    @ObservationIgnored private let engine: ScriviEngine
    @ObservationIgnored private let projectRootPath: String
    /// The whole `project-settings.json` as read — including keys this build does not know.
    @ObservationIgnored private var document: [String: Any] = [:]
    /// Why the settings file could not be read (nil when it could). ⚠️ The core never repairs it on read;
    /// this class does not overwrite it on open either — only an explicit change by the writer does.
    private(set) var unreadableMessage: String?

    // Writing surface
    var showChapterTitles: Bool {
        didSet { save() }
    }

    // Project identity. The title is `project.json`'s; a rename is written there through the core.
    var projectTitle: String {
        didSet { writeTitle() }
    }

    var projectSubtitle: String {
        didSet { save() }
    }

    // ✅ EP-047 S2 (P2, P5a, P6, P8): the manuscript's typeface (a BUNDLED face's name) and text size, per project.
    // ABSENT from the file = the defaults (Literata, 16 pt) — nothing is written until the writer chooses, so a project
    // opened by an older Scrivi keeps its file untouched. ⚠️ A name this build does not bundle is KEPT; only the drawing
    // falls back (`BundledFonts.face(named:)`).
    var typeface: String {
        didSet { typefaceChosen = true; save() }
    }

    var textSize: Double {
        didSet { textSizeChosen = true; save() }
    }
    // ✅ EP-047 S3 (P3, P10): the first-line indent — `book` · `every` · `none` (absent = `book`) and its amount in ems
    // (absent = 1.5). Same rule as the type: written only once the writer chooses.
    var paragraphIndent: String {
        didSet { paragraphIndentChosen = true; save() }
    }

    var indentEm: Double {
        didSet { indentEmChosen = true; save() }
    }

    /// Whether the file holds an explicit choice — so a save of ANOTHER setting never pins today's default into it.
    @ObservationIgnored private var typefaceChosen = false
    @ObservationIgnored private var textSizeChosen = false
    @ObservationIgnored private var paragraphIndentChosen = false
    @ObservationIgnored private var indentEmChosen = false

    /// The type the manuscript draws with.
    var typography: ManuscriptTypography {
        ManuscriptTypography(faceName: typeface, size: CGFloat(textSize),
                             indent: ManuscriptTypography.ParagraphIndent(rawValue: paragraphIndent) ?? ManuscriptTypography.defaultIndent,
                             indentEm: CGFloat(indentEm))
    }

    static let subtitleKey = "subtitle"
    static let showChapterTitlesKey = "showChapterTitles"
    static let typefaceKey = "typeface"
    static let textSizeKey = "textSize"
    static let paragraphIndentKey = "paragraphIndent"
    static let indentEmKey = "indentEm"

    init(projectID: String, projectRootPath: String, schemaTitle: String, engine: ScriviEngine,
         defaults: UserDefaults = .standard) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        let fetch = try? engine.getProjectSettings(projectRootPath: projectRootPath)
        var doc: [String: Any] = [:]
        if fetch?.status == .ok, let json = fetch?.documentJSON,
           let obj = (try? JSONSerialization.jsonObject(with: Data(json.utf8))) as? [String: Any] {
            doc = obj
        }
        var title = schemaTitle

        // ✅ [I-0278] migration — RULED 2026-10-07 (SP-167 Q2): the PACKAGE wins for subtitle and "Show chapter
        // titles"; a title renamed on this Mac is written to `project.json` ONCE — only while the package has no
        // settings yet; after that `project.json` wins. The old key is deleted once migrated.
        // ⛔ Never while the settings file is UNREADABLE: the writer's damaged file may still be recoverable,
        // and this Mac's values would overwrite it. The key is kept for a later open.
        let key = Self.legacyKey(for: projectID)
        if let data = defaults.data(forKey: key), fetch?.status != .unreadable,
           let old = try? JSONDecoder().decode(LegacyStored.self, from: data) {
            var migrated = true
            if fetch?.status == .absent {
                doc[Self.subtitleKey] = old.projectSubtitle
                doc[Self.showChapterTitlesKey] = old.showChapterTitles
                migrated = Self.put(doc, engine: engine, root: projectRootPath)
                let renamed = old.projectTitle.trimmingCharacters(in: .whitespacesAndNewlines)
                if migrated, !renamed.isEmpty, renamed != schemaTitle,
                   (try? engine.setProjectTitle(projectRootPath: projectRootPath, title: renamed)) != nil {
                    title = renamed
                }
            }
            if migrated { defaults.removeObject(forKey: key) }
        }

        document = doc
        unreadableMessage = fetch?.status == .unreadable ? (fetch?.message ?? "unreadable") : nil
        showChapterTitles = doc[Self.showChapterTitlesKey] as? Bool ?? false
        projectSubtitle = doc[Self.subtitleKey] as? String ?? ""
        typeface = doc[Self.typefaceKey] as? String ?? BundledFonts.manifest.default
        textSize = (doc[Self.textSizeKey] as? NSNumber)?.doubleValue ?? Double(ManuscriptTypography.defaultSize)
        paragraphIndent = doc[Self.paragraphIndentKey] as? String ?? ManuscriptTypography.defaultIndent.rawValue
        indentEm = (doc[Self.indentEmKey] as? NSNumber)?.doubleValue ?? Double(ManuscriptTypography.defaultIndentEm)
        typefaceChosen = doc[Self.typefaceKey] != nil
        textSizeChosen = doc[Self.textSizeKey] != nil
        paragraphIndentChosen = doc[Self.paragraphIndentKey] != nil
        indentEmChosen = doc[Self.indentEmKey] != nil
        projectTitle = title
    }

    private func save() {
        document[Self.subtitleKey] = projectSubtitle
        document[Self.showChapterTitlesKey] = showChapterTitles
        if typefaceChosen { document[Self.typefaceKey] = typeface }
        if textSizeChosen { document[Self.textSizeKey] = textSize }
        if paragraphIndentChosen { document[Self.paragraphIndentKey] = paragraphIndent }
        if indentEmChosen { document[Self.indentEmKey] = indentEm }
        if Self.put(document, engine: engine, root: projectRootPath) { unreadableMessage = nil }
    }

    private func writeTitle() {
        // ⛔ An empty title is not written (the core refuses it; the display shows "Untitled").
        let t = projectTitle.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !t.isEmpty else { return }
        do {
            try engine.setProjectTitle(projectRootPath: projectRootPath, title: t)
        } catch {
            NSLog("[ProjectPreferences] title not saved: \(error)")
        }
    }

    private static func put(_ doc: [String: Any], engine: ScriviEngine, root: String) -> Bool {
        guard let data = try? JSONSerialization.data(withJSONObject: doc, options: [.sortedKeys, .prettyPrinted]),
              let json = String(data: data, encoding: .utf8) else { return false }
        do {
            return try engine.putProjectSettings(projectRootPath: root, documentJson: json).saved
        } catch {
            NSLog("[ProjectPreferences] settings not saved: \(error)")
            return false
        }
    }

    // The pre-SP-167 `UserDefaults` record — read only to migrate it.
    static func legacyKey(for projectID: String) -> String {
        "scrivi.project.\(projectID).preferences"
    }

    struct LegacyStored: Codable {
        var showChapterTitles: Bool
        var projectTitle:      String
        var projectSubtitle:   String
    }
}
