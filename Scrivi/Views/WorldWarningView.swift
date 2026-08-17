//
//  WorldWarningView.swift
//  Scrivi
//
//  EP-031 AC23/AC24 (SP-102 / T-0389) — the project-wide warning surface.
//
//  Doc 2 §7.3 / Doc 3 §4.6: a passive, non-blocking report that a bound world is
//  unavailable and edges are held pending. The per-card footer says which world a
//  *card* is waiting on; this says what the *project* is waiting on, which is the only
//  place a writer can see the whole picture at once.
//
//  ⚠️ ANCHORING (SP-102 R1, user-ruled 2026-08-17; Doc 3 §4.6 amended to match)
//
//  Doc 3 originally placed this "beneath the Timeline view". The Timeline is
//  hidden by default and toggled separately, so anchoring to it would make the only
//  project-wide pending report invisible exactly when the writer is not using the
//  Timeline — i.e. most of the time. It therefore has its OWN visibility, and
//  auto-reveals when a world goes away.
//
//  ⚠️ NOTHING DESTRUCTIVE LIVES HERE. "Remove All World References" belongs to the
//  Worlds menu and must be sought out deliberately (§7.3). Putting a destructive
//  global action inside the surface that appears automatically when something looks
//  broken is precisely the hazard Doc 3 §4.6 exists to prevent — and the writer is
//  seeing this strip at her least confident moment.
//

import SwiftUI

@Observable @MainActor final class WorldWarningModel {

    /// One unavailable world and what is waiting on it.
    struct Row: Identifiable, Sendable {
        let worldID:     String
        let displayName: String
        let status:      WorldStatus
        /// Edges held pending toward this world. ⚠️ This is what the writer would
        /// lose if anything pruned them — which is why it is stated as a count.
        let pendingCount: Int

        var id: String { worldID }
    }

    private(set) var rows: [Row] = []
    /// Set when the report itself could not be produced. Shown rather than swallowed:
    /// a silent failure here means a writer believes nothing is pending.
    private(set) var loadError: String?

    /// Whether the writer has dismissed the strip for this session. Dismissal is
    /// per-appearance, not permanent — a NEW world going unavailable re-reveals it.
    var dismissed = false

    private var lastSeenWorldIDs: Set<String> = []

    func reload(engine: ScriviEngine, projectRootPath: String) {
        guard !projectRootPath.isEmpty else { rows = []; return }
        do {
            let worlds = try engine.listWorlds(projectRootPath: projectRootPath).worlds
            let pending = try engine.listPendingEdges(projectRootPath: projectRootPath).pending

            // Count pending edges per world so each row can say what is at stake.
            var counts: [String: Int] = [:]
            for edge in pending { counts[edge.worldID, default: 0] += 1 }

            // ⚠️ Report every unavailable BOUND world, not merely those with pending
            // edges. A world that is away with nothing yet linked is still something
            // the writer needs to know about — she cannot add to it, and silence
            // would read as "everything is fine".
            //
            // `worldStatus` carries the AC24 platform refinement (WorldVolumeStatus),
            // so this reports `unmounted`/`offline` without restating that logic.
            rows = worlds
                .filter { $0.worldStatus.isUnavailable }
                .map { world in
                    Row(worldID: world.worldID,
                        displayName: world.displayName.isEmpty ? world.worldID : world.displayName,
                        status: world.worldStatus,
                        pendingCount: counts[world.worldID] ?? 0)
                }
                .sorted { $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending }

            // Auto-reveal when a world we had not previously flagged goes away.
            let ids = Set(rows.map(\.worldID))
            if !ids.subtracting(lastSeenWorldIDs).isEmpty { dismissed = false }
            lastSeenWorldIDs = ids

            loadError = nil
        } catch {
            rows = []
            loadError = error.localizedDescription
        }
    }

    /// The strip shows only when there is something to report and it has not been
    /// dismissed. Nothing pending means no strip at all — it must not become
    /// furniture the writer learns to ignore.
    var isVisible: Bool { (!rows.isEmpty || loadError != nil) && !dismissed }
}

struct WorldWarningView: View {
    let model: WorldWarningModel
    /// Opens the Worlds menu, where the repair affordances (Locate…, Remove) live.
    /// ⚠️ This view *routes* to repair; it never performs one.
    var onManageWorlds: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(.orange)
                    .accessibilityHidden(true)
                Text(headline)
                    .font(.caption).fontWeight(.medium)

                Spacer(minLength: 8)

                Button("Manage Worlds…", action: onManageWorlds)
                    .font(.caption)
                    .buttonStyle(.borderless)

                Button {
                    model.dismissed = true
                } label: {
                    Image(systemName: "xmark")
                        .font(.caption2)
                }
                .buttonStyle(.borderless)
                .foregroundStyle(.secondary)
                .help("Hide this notice. It returns if another world becomes unavailable.")
                .accessibilityLabel("Hide world warning")
            }

            if let error = model.loadError {
                Text(error)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            ForEach(model.rows) { row in
                Text(sentence(for: row))
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.orange.opacity(0.10))
        .overlay(alignment: .top) { Divider() }
        // Non-blocking by construction: it is a strip in the layout, never a sheet,
        // never an alert, and it never takes focus from the manuscript.
        .accessibilityElement(children: .contain)
    }

    private var headline: String {
        model.rows.count == 1
            ? "1 world is unavailable"
            : "\(model.rows.count) worlds are unavailable"
    }

    /// ⚠️ Says "held pending", never anything that implies loss — the links ARE
    /// intact (AC7), and the whole purpose of this surface is to say so before the
    /// writer concludes otherwise and reaches for a destructive remedy (I-0115).
    private func sentence(for row: WorldWarningModel.Row) -> String {
        let links = row.pendingCount == 0
            ? "No links are affected yet"
            : (row.pendingCount == 1
                ? "1 link is held pending"
                : "\(row.pendingCount) links are held pending")
        return "“\(row.displayName)” is \(row.status.writerDescription). \(links)."
    }
}
