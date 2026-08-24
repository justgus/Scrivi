import SwiftUI
#if os(macOS)
import AppKit
#endif

/// The Detail Sheet's image section — import, display, replace, remove
/// (EP-034 SP-119, T-0447; **AC3**).
///
/// ## ⚠️ The whole asset stack shipped in SP-116 with no caller
///
/// `importAsset` / `listAssets` / `removeAsset`, world-scoped storage (D6) and
/// `assetPath` resolution (D7) were all built and Verified — and
/// `grep -rn "importAsset" Scrivi/Views/` returned **nothing**. ⚠️ **That is the
/// correct build order for a cross-platform app** (ScriviCore is the capability
/// layer, this is one surface over it) — but it does mean this view is the first
/// thing to exercise any of it.
///
/// ## ⚠️ A world-scoped object's image lives in the WORLD package
///
/// D6: passing `worldID` puts the bytes inside `.scrivworld`, so the image
/// travels with the world and is present when that world is opened **from a
/// different project** (AC3). ⚠️ **Passing `""` for a world-scoped object is
/// I-0151's defect** — `kindDirFor` fails outright rather than defaulting.
///
/// ## ⚠️ Fields deliberately NOT surfaced
///
/// Decided before this view was written (S11), not discovered afterwards:
/// `assetID` is plumbing and never shown; `title` is set to the filename and not
/// separately editable; `category` is always `image`; `sidecarPath` never
/// appears. `filename` **is** shown — quietly — because it is the only
/// human-recognisable link back to the file the writer chose.
struct ObjectImageSection: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// The object being edited — its `worldID` decides where bytes land.
    let objectID: String
    let objectKind: String
    let worldID: String
    let projectID: String
    let authorshipRef: AuthorshipRef
    /// ⚠️ Read-only when the object's world is away: the graph and the package
    /// are frozen in both directions (Doc 3 §4.6).
    let isReadOnly: Bool
    /// The image currently on the object, resolved by the core. Empty when there
    /// is none — or when it cannot be resolved right now.
    let imagePath: String?
    let imageAssetID: String?
    /// Announces a change so the sheet re-reads and the host refreshes the
    /// inspector (I-0155/I-0160's mechanism).
    let onChanged: () -> Void

    @State private var actionError: String?
    @State private var isWorking = false
    /// I-0164 / T-0451 — the "already in this world" picker.
    @State private var showExisting = false

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Image")
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)

            if let path = resolvedPath {
                preview(path)
            } else if hasUnresolvableImage {
                // ⚠️ The object HAS an image but its bytes are unreachable — the
                // world is away. Say so: an empty frame here would read as "your
                // picture is gone" (Doc 3 — absence is never deletion).
                Label("This object's image is in a world that is unavailable.",
                      systemImage: "photo.badge.exclamationmark")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .padding(8)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
            } else {
                Text("No image.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            }

            controls

            if let actionError {
                Label(actionError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }
        }
    }

    // MARK: — Presentation

    @ViewBuilder
    private func preview(_ path: String) -> some View {
        VStack(alignment: .leading, spacing: 3) {
            // ⚠️ Loaded from disk each time rather than cached here. The bytes may
            // sit on a removable volume, so a cache would go stale exactly when it
            // matters — and the sheet shows one image, not a scrolling list.
            // ⚠️ I-0162: check the bytes are actually THERE before blaming the
            // file. `imagePath` is a snapshot from load time and a removable
            // volume can vanish after it — reporting "could not be read from
            // disk" for an ejected drive tells the writer her picture is damaged
            // when it is merely absent.
            if !FileManager.default.fileExists(atPath: path) {
                Label("This object's image is in a world that is unavailable.",
                      systemImage: "photo.badge.exclamationmark")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .padding(8)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
            } else if let image = loadImage(path) {
                image
                    .resizable()
                    .scaledToFit()
                    .frame(maxWidth: .infinity, maxHeight: 220)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
                    .overlay(RoundedRectangle(cornerRadius: 6).stroke(.quaternary))
            } else {
                // The core resolved a path and the bytes would not decode —
                // a damaged or unsupported file. Say which, rather than showing
                // a blank frame.
                Label("This image could not be read from disk.",
                      systemImage: "photo.badge.exclamationmark")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            // S11: the filename is shown — quietly — because it is the only
            // human-recognisable link back to the file she chose. ⚠️ The assetID
            // is NOT shown; it means nothing to a writer.
            //
            // ⚠️ Suppressed when the bytes are unreachable: a filename under an
            // outage message reads as though the file is present.
            if FileManager.default.fileExists(atPath: path) {
                Text((path as NSString).lastPathComponent)
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
        }
    }

    @ViewBuilder
    private var controls: some View {
        HStack(spacing: 8) {
            Button(hasImage ? "Replace…" : "Add Image…") { pickAndImport() }
                .disabled(isReadOnly || isWorking)
                // ⚠️ Disabled AND explained (§7.2), never merely greyed out.
                .help(isReadOnly
                      ? "This object's world is unavailable, so its image cannot be changed."
                      : (hasImage ? "Choose a different image" : "Choose an image for this object"))

            // ⚠️ T-0451 (I-0164, user-ruled): reach an asset ALREADY in the world.
            //
            // Without this the only way to set an image was to import from the
            // filesystem — so a picture already sitting in the world package was
            // unreachable, and the workaround (re-import the same filename)
            // OVERWRITES its sidecar and orphans the original assetID.
            // ⚠️ **This removes the REASON to re-import; it does not fix the
            // orphaning**, which remains a known limitation with a test pinning it.
            Button("Choose Existing…") { showExisting = true }
                .disabled(isReadOnly || isWorking)
                .help(isReadOnly
                      ? "This object's world is unavailable, so its image cannot be changed."
                      : "Use an image already in this world")
                .popover(isPresented: $showExisting, arrowEdge: .bottom) {
                    ExistingAssetPicker(
                        engine: engine,
                        projectRootPath: projectRootPath,
                        worldID: worldID,
                        currentAssetID: imageAssetID,
                        onPick: { assetID in
                            showExisting = false
                            attachExisting(assetID: assetID)
                        }
                    )
                }

            if hasImage {
                Button("Remove", role: .destructive) { removeImage() }
                    .disabled(isReadOnly || isWorking)
                    .help(isReadOnly
                          ? "This object's world is unavailable, so its image cannot be removed."
                          : "Remove this image from the object")
            }

            if isWorking { ProgressView().controlSize(.small) }
        }
        .font(.caption)
        .buttonStyle(.borderless)
    }

    // MARK: — State

    private var resolvedPath: String? {
        guard let p = imagePath, !p.isEmpty else { return nil }
        return p
    }

    private var hasImage: Bool {
        !(imageAssetID ?? "").isEmpty
    }

    /// The object names an image the core could not resolve to a path — its world
    /// is away. ⚠️ Distinct from "no image": one is absence, the other is an
    /// outage, and they must not read the same.
    private var hasUnresolvableImage: Bool {
        hasImage && resolvedPath == nil
    }

    private func loadImage(_ path: String) -> Image? {
        #if os(macOS)
        guard let nsImage = NSImage(contentsOfFile: path) else { return nil }
        return Image(nsImage: nsImage)
        #else
        guard let ui = UIImage(contentsOfFile: path) else { return nil }
        return Image(uiImage: ui)
        #endif
    }

    // MARK: — Actions

    private func pickAndImport() {
        #if os(macOS)
        let panel = NSOpenPanel()
        panel.title = "Choose an Image"
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        panel.allowedContentTypes = [.image]
        guard panel.runModal() == .OK, let url = panel.url else { return }
        importImage(from: url)
        #endif
    }

    private func importImage(from url: URL) {
        isWorking = true
        defer { isWorking = false }

        // A user-picked file outside the sandbox needs scoped access for the copy.
        // ⚠️ No bookmark is recorded: the bytes are copied INTO the package, so the
        // original is never read again — unlike a world package, which is
        // referenced for the life of the project.
        let scoped = url.startAccessingSecurityScopedResource()
        defer { if scoped { url.stopAccessingSecurityScopedResource() } }

        do {
            // ⚠️ Import BEFORE detaching the old image. If this fails, the object
            // keeps the picture it had — a failed replace must never leave the
            // writer with nothing.
            let imported = try engine.importAsset(
                projectRootPath: projectRootPath,
                sourcePath: url.path(percentEncoded: false),
                category: "image",
                // S11: title is the filename. The core accepts a separate title;
                // a second editable name at this surface was deliberately not
                // added, and the field stays available for captions later.
                title: url.lastPathComponent,
                authorshipRef: authorshipRef,
                // ⚠️ D6/AC3: a world-scoped object's bytes go in the WORLD package.
                worldID: worldID,
                projectID: projectID
            )

            let previous = imageAssetID
            try attachImage(assetID: imported.assetID)

            // Only now is the old asset safe to delete — the object already points
            // at the new one, so a failure here leaves an orphaned file rather
            // than an object pointing at bytes that are gone.
            if let previous, !previous.isEmpty, previous != imported.assetID {
                _ = try? engine.removeAsset(
                    projectRootPath: projectRootPath,
                    assetID: previous, worldID: worldID, projectID: projectID)
            }

            actionError = nil
            onChanged()
        } catch {
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    /// Points the object at an asset that is ALREADY in the package.
    ///
    /// ⚠️ **No import, so no new assetID** — that is the entire point (S12). A new
    /// ID would mean the bytes were copied again, which is the defect I-0164
    /// describes.
    ///
    /// ⚠️ The previous asset is deliberately NOT deleted here. It may be shared:
    /// nothing stops two objects referencing one image, and `removeAsset` would
    /// take the bytes out from under the other one. ⚠️ **Import's replace path
    /// does delete, because there it created the replacement itself.**
    private func attachExisting(assetID: String) {
        guard assetID != (imageAssetID ?? "") else { return }
        isWorking = true
        defer { isWorking = false }
        do {
            try attachImage(assetID: assetID)
            actionError = nil
            onChanged()
        } catch {
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    private func removeImage() {
        guard let assetID = imageAssetID, !assetID.isEmpty else { return }
        isWorking = true
        defer { isWorking = false }

        do {
            // Detach FIRST, delete second — the reverse order would leave the
            // object pointing at bytes that no longer exist if the second step
            // failed.
            try attachImage(assetID: nil)
            _ = try? engine.removeAsset(
                projectRootPath: projectRootPath,
                assetID: assetID, worldID: worldID, projectID: projectID)
            actionError = nil
            onChanged()
        } catch {
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    /// Writes (or clears) `image.assetID` on the object.
    ///
    /// ⚠️ **Re-reads from disk and PATCHES** — never reconstructs. I-0155: the
    /// Scene Inspector can be editing the same object, and patching a stale
    /// snapshot silently reverts whatever it wrote. `ObjectCardModel.rename` has
    /// always done it this way.
    private func attachImage(assetID: String?) throws {
        let opened = try engine.openObject(
            projectRootPath: projectRootPath,
            objectKind: objectKind,
            objectID: objectID,
            worldID: worldID
        )
        guard var json = try JSONSerialization.jsonObject(
            with: Data(opened.objectJson.utf8)) as? [String: Any] else {
            throw ScriviError(code: -1, message: "Could not read this object.")
        }

        if let assetID, !assetID.isEmpty {
            // ⚠️ Preserve any sibling keys (`thumbnailAssetID`) rather than
            // replacing the sub-object wholesale — nothing generates thumbnails
            // yet, but the read path already prefers one (T-0446).
            var image = (json["image"] as? [String: Any]) ?? [:]
            image["assetID"] = assetID
            json["image"] = image
        } else {
            json.removeValue(forKey: "image")
        }

        let patched = try JSONSerialization.data(withJSONObject: json)
        _ = try engine.saveObject(
            projectRootPath: projectRootPath,
            objectKind: objectKind,
            objectJson: String(decoding: patched, as: UTF8.self),
            authorshipRef: authorshipRef
        )
    }
}
