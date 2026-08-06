import SwiftUI

// T-0367 — the Properties tab.
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §4.8, §4.8.1, AC14–AC15.
//
// NOT a card stack (AC14): a fixed view of facts about the scene.
//
// This is the one legitimate exception to §2's manual-surfacing principle — §2 governs
// what the app proposes about *worlds and objects*, not whether a file may show its own
// metadata.
//
// FIELD-DRIVEN by construction (§4.8.1, AC15): every row is a `SceneProperty` that
// declares its own editability, so adding an editable property later is a field-level
// change rather than a rewrite. Read-only content today — but deliberately not a
// read-only architecture.

/// Why a property cannot be edited. The two cases diverge as soon as the first editable
/// property lands, so they are distinguished from the start rather than collapsed into
/// one "read-only" flag.
enum PropertyEditability: Equatable {
    /// Derived from other data — editing it would be meaningless (metrics, timestamps).
    case derived
    /// Editable in principle; this build has no editor for it yet.
    case noEditorYet
    /// Editable now, writing through the supplied action.
    case editable
}

struct SceneProperty: Identifiable {
    let id: String
    let label: String
    let value: String
    let editability: PropertyEditability
    /// Set only when `editability == .editable`. Unused today; present so the row
    /// renderer already has the hook.
    var commit: ((String) -> Void)? = nil

    var isEditable: Bool { editability == .editable }
}

struct ScenePropertiesView: View {

    let sceneID: String?
    let projectRootPath: String?
    let engine: ScriviEngine

    @State private var notes: SceneNotesResult?
    @State private var loadFailed = false

    var body: some View {
        Group {
            if sceneID == nil {
                message("No scene selected.")
            } else if loadFailed {
                message("Couldn't read this scene's properties.")
            } else if let notes {
                ScrollView {
                    VStack(alignment: .leading, spacing: 0) {
                        section("Scene", properties(for: notes).filter { $0.id.hasPrefix("scene.") })
                        section("Authorship", properties(for: notes).filter { $0.id.hasPrefix("author.") })
                        section("Metrics", properties(for: notes).filter { $0.id.hasPrefix("metric.") })
                    }
                    .padding(12)
                }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        .task(id: sceneID) { load() }
    }

    // MARK: — Fields

    private func properties(for n: SceneNotesResult) -> [SceneProperty] {
        [
            SceneProperty(id: "scene.title", label: "Title",
                          value: n.title.isEmpty ? "Untitled" : n.title,
                          // The title IS editable — via the Scene Navigator's rename,
                          // which owns the file-rename side effects. Not duplicated here.
                          editability: .noEditorYet),
            SceneProperty(id: "scene.id", label: "Scene ID",
                          value: n.sceneID, editability: .derived),

            SceneProperty(id: "author.createdBy", label: "Author",
                          value: n.createdByDisplayName.isEmpty ? "—" : n.createdByDisplayName,
                          editability: .derived),
            SceneProperty(id: "author.createdAt", label: "Created",
                          value: format(n.createdAt), editability: .derived),
            SceneProperty(id: "author.modifiedBy", label: "Last edited by",
                          value: n.modifiedByDisplayName.isEmpty ? "—" : n.modifiedByDisplayName,
                          editability: .derived),
            SceneProperty(id: "author.modifiedAt", label: "Modified",
                          value: format(n.modifiedAt), editability: .derived),

            SceneProperty(id: "metric.words", label: "Words",
                          value: "\(n.wordCount)", editability: .derived),
            SceneProperty(id: "metric.characters", label: "Characters",
                          value: "\(n.characterCount)", editability: .derived),
        ]
    }

    // MARK: — Rendering

    @ViewBuilder
    private func section(_ title: String, _ rows: [SceneProperty]) -> some View {
        if !rows.isEmpty {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
                .padding(.bottom, 4)

            VStack(spacing: 6) {
                ForEach(rows) { PropertyRow(property: $0) }
            }
            .padding(.bottom, 14)
        }
    }

    private func message(_ text: String) -> some View {
        VStack {
            Spacer()
            Text(text).font(.callout).foregroundStyle(.secondary)
            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: — Loading

    private func load() {
        guard let sceneID, let projectRootPath else { return }
        do {
            notes = try engine.getSceneNotes(projectRootPath: projectRootPath, sceneID: sceneID)
            loadFailed = false
        } catch {
            loadFailed = true
        }
    }

    /// ISO-8601 → a form a writer reads. Falls back to the raw string rather than
    /// showing nothing when the stored value is not parseable.
    private func format(_ iso: String) -> String {
        guard !iso.isEmpty else { return "—" }
        let parser = ISO8601DateFormatter()
        parser.formatOptions = [.withInternetDateTime, .withFractionalSeconds]
        let date = parser.date(from: iso) ?? ISO8601DateFormatter().date(from: iso)
        guard let date else { return iso }
        let out = DateFormatter()
        out.dateStyle = .medium
        out.timeStyle = .short
        return out.string(from: date)
    }
}

private struct PropertyRow: View {
    let property: SceneProperty

    var body: some View {
        HStack(alignment: .firstTextBaseline, spacing: 8) {
            Text(property.label)
                .font(.caption)
                .foregroundStyle(.secondary)
                .frame(width: 92, alignment: .leading)

            Text(property.value)
                .font(.callout)
                .textSelection(.enabled)
                .frame(maxWidth: .infinity, alignment: .leading)
        }
    }
}
