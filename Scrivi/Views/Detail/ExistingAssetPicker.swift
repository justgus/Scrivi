import SwiftUI
#if os(macOS)
import AppKit
#endif

/// Picks an image **already in the package** so it can be attached to an object
/// without importing anything (EP-034 SP-119, T-0451; closes **I-0164**).
///
/// ## ⚠️ Why this exists
///
/// T-0447 shipped exactly one way to set an image — *Add Image…*, which imports
/// from the filesystem. ⚠️ **A picture already sitting in the world was therefore
/// unreachable**, which is precisely what happened on the rig: `myton-portrait.png`
/// was in `Eskandar.scrivworld` and Myton's record had no `image` key, so the app
/// could not show it and offered no way to link it.
///
/// ⚠️ **And the obvious workaround is harmful.** `AssetStore` stores bytes at
/// `assets/<category>/<filename>` with the sidecar at `<filename>.meta.json`, so
/// re-importing the same filename **overwrites both and mints a new assetID** —
/// the original becomes unresolvable and any object still pointing at it dangles
/// silently. ✅ **Proven by test (I-0164).**
///
/// ⚠️ **This view removes the REASON to re-import. It does not fix the
/// orphaning**, which remains a known limitation — the user ruled option 1
/// (attach) over option 2 (idempotent import) and option 3 (assetID-named files).
///
/// ## ⚠️ Attaching is not importing
///
/// Picking here writes the **existing** `assetID` onto the object. No bytes are
/// copied and no new identity is minted — S12 asserts exactly that, because a new
/// ID would mean an import happened.
struct ExistingAssetPicker: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// Which package to look in. Empty = the project's own assets.
    let worldID: String
    /// Shown with a checkmark and not re-pickable.
    let currentAssetID: String?
    let onPick: (String) -> Void

    @State private var assets: [ListedAsset] = []
    @State private var loadError: String?
    @State private var didLoad = false

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Images in this world")
                .font(.headline)

            if let loadError {
                Label(loadError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            if assets.isEmpty {
                // ⚠️ Distinguish "none yet" from "not loaded" — an empty box that
                // might mean either is the kind of thing a writer reads as broken.
                Text(didLoad
                     ? "No images have been imported into this world yet."
                     : "Loading…")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .frame(minHeight: 120, alignment: .top)
            } else {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 2) {
                        ForEach(assets, id: \.assetID) { asset in
                            row(asset)
                        }
                    }
                }
                // ⚠️ I-0127's lesson: a MINIMUM is what reserves space inside a
                // popover, which sizes to its content. A bare maximum collapses
                // the list to a sliver.
                .frame(minHeight: 140, maxHeight: 300)
            }
        }
        .padding(12)
        .frame(width: 320)
        .task { load() }
    }

    @ViewBuilder
    private func row(_ asset: ListedAsset) -> some View {
        let isCurrent = asset.assetID == (currentAssetID ?? "")
        Button {
            onPick(asset.assetID)
        } label: {
            HStack(spacing: 8) {
                thumbnail(asset.assetPath)

                VStack(alignment: .leading, spacing: 1) {
                    // The asset's title, falling back to its filename — ⚠️ never
                    // the assetID, which means nothing to a writer.
                    Text(asset.title.isEmpty ? asset.filename : asset.title)
                        .font(.callout)
                        .lineLimit(1)
                    if !asset.title.isEmpty && asset.title != asset.filename {
                        Text(asset.filename)
                            .font(.caption2)
                            .foregroundStyle(.tertiary)
                            .lineLimit(1)
                            .truncationMode(.middle)
                    }
                }

                Spacer(minLength: 4)

                if isCurrent {
                    Image(systemName: "checkmark")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.borderless)
        .disabled(isCurrent)
        .help(isCurrent ? "Already this object's image" : "Use this image")
    }

    /// A small preview. ⚠️ **A failed load is expected, not corruption** — the
    /// bytes may be on a volume that has just gone away — so this degrades to an
    /// icon rather than reporting an error.
    @ViewBuilder
    private func thumbnail(_ path: String) -> some View {
        #if os(macOS)
        if FileManager.default.fileExists(atPath: path),
           let nsImage = NSImage(contentsOfFile: path) {
            Image(nsImage: nsImage)
                .resizable()
                .scaledToFill()
                .frame(width: 32, height: 32)
                .clipShape(RoundedRectangle(cornerRadius: 4))
        } else {
            Image(systemName: "photo")
                .font(.caption)
                .foregroundStyle(.tertiary)
                .frame(width: 32, height: 32)
        }
        #else
        Image(systemName: "photo")
            .font(.caption)
            .foregroundStyle(.tertiary)
            .frame(width: 32, height: 32)
        #endif
    }

    private func load() {
        do {
            // ⚠️ Images only. The core stores audio/video/documents in the same
            // package and none of them can be an object's image, so listing them
            // would offer choices that cannot work.
            assets = try engine.listAssets(
                projectRootPath: projectRootPath,
                category: "image",
                worldID: worldID
            ).assets
            loadError = nil
        } catch {
            assets = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
        didLoad = true
    }
}
