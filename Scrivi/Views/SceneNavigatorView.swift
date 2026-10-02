import SwiftUI

// SceneNavigatorView — sidebar list of all scenes in manuscript order.
//
// Chapters are rendered as non-selectable header rows in a flat ForEach,
// which lets SwiftUI's built-in List reorder handle both scene and chapter
// drag-reorder in a single onMove callback.
//
// Right-click (macOS) / long-press (iOS) on a row opens a context menu
// with Rename and Delete actions.
/// A request to scroll a scene into view (I-0161).
struct SceneRevealRequest: Equatable {
    let sceneID: String
    /// Distinguishes two requests for the same scene.
    let token: Int
}

struct SceneNavigatorView: View {

    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var session: ProjectSession
    // ⚠️ T-0544: `projectHeader` was this property's ONLY reader, and it is gone. `prefs` is
    // KEPT deliberately — ⛔ removing it would mean changing the initialiser and both platform
    // call sites in `EditorView`, which is churn beyond [I-0243]'s scope, and a navigator that
    // knows its project's preferences is a plausible near-term need.
    var prefs: ProjectPreferences
    // macOS: tap-to-navigate within the continuous manuscript. Unused on iOS, where the List
    // selection binding (`selection`) is the navigation source of truth so NavigationSplitView
    // can drive the detail column. Defaults to a no-op so iOS call sites can omit it.
    var onNavigate: (String) -> Void = { _ in }
    var onTakeFocus: () -> Void        // called after delete to transfer first-responder
    // iOS Master/Detail selection (bare sceneID). When provided, the List selection binds to it
    // so master and detail stay in sync and the compact-width detail push is selection-driven.
    var selection: Binding<String?>? = nil
    /// I-0161 — a scene the writer was sent to from ANOTHER surface, which must be
    /// scrolled into view. ⚠️ Deliberately distinct from `selection`: selecting a
    /// row she clicked herself must NOT move the list (I-0132).
    /// ⚠️ Carries a token as well as the ID, so navigating to the SAME scene twice
    /// still fires. A bare `String?` would be an unchanged value the second time
    /// and SwiftUI would coalesce it away — the exact shape of I-0132's
    /// one-shot-trigger bug, which cost three failed attempts to diagnose.
    var revealRequest: SceneRevealRequest? = nil

    // Rename sheet state
    /// A scene asked for from another surface, awaiting the viewport catching up
    /// (I-0161). ⚠️ Cleared on arrival so this reveals ONCE and never fights the
    /// writer's own scrolling afterwards.
    @State private var pendingReveal: String? = nil
    @State private var renameTarget: RenameTarget? = nil
    // Delete confirmation state
    @State private var deleteSceneTarget: SceneEntry? = nil
    @State private var deleteChapterTarget: ChapterGroup? = nil
    // Error alert state
    @State private var alertError: String? = nil
    // The scene ID currently highlighted in the list — driven only by viewportSceneID.
    // Never drives navigation; navigation is triggered by explicit tap gestures only.
    @State private var highlightedRowID: String? = nil
    // T-0569 — the navigator search field's text.
    @State private var searchText: String = ""
    // T-0569 — scenes whose BODY matched, tagged with the query that produced them so a
    // result still in flight for an older query is never applied to a newer one.
    @State private var bodyMatches: BodyMatches? = nil

    var body: some View {
        // ⚠️ T-0544 / [I-0243]: there is NO project-title header here any more. The title is the
        // WINDOW's (`.navigationTitle`), rendered once, by the platform.
        // ⛔ Do not reintroduce one — it was the third of three renderings of the same string.
        VStack(spacing: 0) {
            navigatorList
        }
    }

    // ⛔ T-0544 / [I-0243]: `projectHeader` WAS HERE and is DELETED.
    //
    // It drew the project title above the scene list, pinned so it never scrolled — the THIRD
    // simultaneous rendering of that one string, alongside the window's `.navigationTitle` and
    // the tab header (the latter removed in the same Task by `tabbingMode = .disallowed`).
    // ⚠️ Three mechanisms, one fact, which is why no single deletion fixed it.
    //
    // ⛔ Do not reintroduce it. The window owns the project's identity.

    // List selection bound to rowID ("scene-<id>"). On iOS this maps to/from the parent's bare
    // sceneID `selection` so the List drives Master/Detail; on macOS it tracks viewportSceneID
    // for highlight only (navigation is via tap there).
    private var listSelection: Binding<String?> {
        guard let selection else { return $highlightedRowID }
        return Binding<String?>(
            get: {
                guard let id = selection.wrappedValue else { return nil }
                return "scene-" + id
            },
            set: { (newRowID: String?) in
                guard let rowID = newRowID, rowID.hasPrefix("scene-") else {
                    selection.wrappedValue = nil
                    return
                }
                let sceneID = String(rowID.dropFirst("scene-".count))
                // T-0571 — written BEFORE the selection, which is what drives the manuscript.
                let query = searchQuery
                loader.searchCaretHint = query.isEmpty ? nil : .init(sceneID: sceneID, query: query)
                selection.wrappedValue = sceneID
            }
        )
    }

    private var navigatorList: some View {
        // ⚠️ **The list must scroll ITSELF to the selected row (I-0134).**
        //
        // Highlighting a row does not reveal it. On restore, and on any navigation to a
        // scene outside the visible slice of the list, the writer saw an apparently
        // empty selection and had to scroll the navigator by hand to find where she
        // actually was. The manuscript centring fixed in I-0131 made this the last
        // remaining half of "go to a scene": the text moved, the list did not.
        ScrollViewReader { proxy in
            List(selection: listSelection) {
                ForEach(flatRows, id: \.rowID) { row in
                    switch row {
                    case .chapterHeader(let group):
                        chapterHeaderRow(for: group)
                            .listRowSeparator(.hidden)
                    case .scene(let entry, let group):
                        sceneRow(for: entry, in: group)
                            // `id` is what `scrollTo` targets; it must match `rowID`,
                            // which is also the List's selection identity.
                            .id(row.rowID)
                    }
                }
                // ⚠️ T-0569 — reorder is OFF while filtering. `performMove` reads the
                // drop's predecessor from `flatRows`, and with rows hidden that predecessor
                // is not the scene's real neighbour, so a drop would land somewhere else.
                .onMove(perform: searchQuery.isEmpty ? { source, destination in
                    performMove(from: source, to: destination)
                } : nil)
            }
            // ✅ T-0569 — always-visible search at the BOTTOM of the navigator, as a
            // `safeAreaBar` like the Inspector's tab bar (T-0545), so it insets the list
            // rather than competing with it for stack space.
            .safeAreaBar(edge: .bottom) {
                VStack(spacing: 0) {
                    Divider()
                    searchField
                }
            }
            // ✅ T-0569 — the body scan runs OFF the main actor (~24 ms on a Bible-sized
            // manuscript, measured 2026-10-01) and is cancelled by the next keystroke.
            // ⚠️ Re-runs when the query or the scene COUNT changes; it deliberately does not
            // observe `segments`, which would re-render the navigator on every keystroke
            // typed in the manuscript.
            .task(id: SearchKey(query: searchQuery, sceneCount: loader.allScenes.count)) {
                let query = searchQuery
                guard !query.isEmpty else { bodyMatches = nil; return }
                // The LIVE text (ruled 2026-09-30): unsaved edits are searched.
                let texts = loader.segments.map { SceneText(sceneID: $0.sceneID, text: $0.text) }
                guard let ids = await Self.scenesMatching(query, in: texts) else { return }
                bodyMatches = BodyMatches(query: query, sceneIDs: ids)
            }
            .onAppear {
                // ⚠️ REVEAL ON LAUNCH ONLY — deliberately not on every selection change.
                //
                // Restore lands before this view is on screen, so the initial highlight
                // needs an explicit reveal; without it the very first thing the writer
                // sees is a selection she cannot find.
                //
                // An `onChange(of: viewportSceneID)` reveal was tried and REMOVED
                // (I-0132, user-reported 2026-08-18). `scrollTo` is not a no-op for an
                // already-visible row: it re-anchors the row to `.center`, so every
                // navigator click nudged the list "a little bit up or down" depending on
                // where the row sat. In practice the writer scrolls the list by hand and
                // clicks a row she can already see, so the reveal had **no** case where it
                // helped and one where it actively fought her. Scene-to-scene reveal is
                // the manuscript's job (`navigateToScene` centres the text), not the
                // list's.
                // ⚠️ SUPERSEDED IN PART 2026-09-30: the list now FOLLOWS the viewport with
                // `anchor: nil` (see the `viewportSceneID` handler below) — ✅ the nudge
                // above was the `.center` anchor's, which a minimal scroll cannot cause.
                if let sceneID = loader.viewportSceneID {
                    proxy.scrollTo("scene-\(sceneID)", anchor: .center)
                }
            }
            // ⚠️ I-0161 — reveal a scene navigated to from OUTSIDE this list.
            //
            // ⚠️ This is NOT the reveal I-0132 removed, and the difference is the
            // whole justification. That one fired on every selection change,
            // including the writer clicking a row she was already looking at, and
            // re-anchoring nudged the list under her hand — it had no case where it
            // helped. **This fires only when something else drove the navigation**
            // (today: double-clicking a scene in the Detail Sheet's related list),
            // where the target is often far off-screen and she has no idea where
            // she landed.
            //
            // ⚠️ `.anchor: nil` scrolls the MINIMUM distance to bring the row into
            // view and does nothing when it is already visible — so it cannot
            // reproduce I-0132's nudge even if the two paths ever overlap.
            .onChange(of: revealRequest) { _, request in
                guard let sceneID = request?.sceneID else { return }

                // ⚠️ Record the request; do NOT scroll here.
                //
                // On macOS this list's highlight is driven ONLY by
                // `loader.viewportSceneID`, which the manuscript publishes AFTER it
                // finishes scrolling. Scrolling now would move the list to a row
                // that is not the current one yet, and the highlight would appear
                // somewhere else once the manuscript caught up — exactly what the
                // writer saw: *"the ManuscriptView moves faithfully, but the Scene
                // Navigator does not."*
                //
                // ⚠️ It is NOT a focus problem, which was the first guess. The
                // reveal was racing the signal that makes the row current.
                pendingReveal = sceneID

                // ⚠️ Expire it. If the manuscript never reports arriving (a failed
                // navigation, or a scene that vanished), a lingering request would
                // fire on some UNRELATED later scroll and yank the list out from
                // under the writer — the same class of surprise I-0132 removed.
                // One navigation, one chance to reveal.
                let requested = sceneID
                DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
                    if pendingReveal == requested { pendingReveal = nil }
                }
            }
            // ⚠️ [T-0568 finding, user ruling 2026-09-30] — THE LIST FOLLOWS THE
            // MANUSCRIPT: whenever the highlighted scene changes, keep its row VISIBLE.
            //
            // ⛔ The writer's report: scrolling the manuscript moved the highlight onto
            // rows BELOW (or above) the list's viewport, and the list never followed —
            // so "where am I?" had no visible answer. `Go to Manuscript End` failed the
            // same way.
            //
            // ⚠️ THIS IS NOT THE REVEAL [I-0132] REMOVED, and the difference is the
            // anchor. That one used `anchor: .center`, which RE-CENTRES a row even when
            // it is already visible — so every click nudged the list. ✅ `anchor: nil`
            // scrolls the MINIMUM distance and does NOTHING for a visible row, so a
            // click on a row she can see cannot move the list.
            // ✅ It keys on `viewportSceneID` — the SAME signal the highlight uses — so
            // the revealed row and the highlighted row can never disagree. (A reveal
            // aimed at a DIFFERENT scene is what broke `Manuscript End`: at the end the
            // viewport's scene can be an earlier one than the last.)
            // ✅ Subsumes the I-0161 deferred reveal below, which is kept only to clear
            // its pending state.
            .onChange(of: loader.viewportSceneID) { _, sceneID in
                pendingReveal = nil
                guard let sceneID else { return }
                proxy.scrollTo("scene-\(sceneID)", anchor: nil)
            }
        }
        .listStyle(.inset)
        .frame(minWidth: 180, idealWidth: 220, maxWidth: 280)
        // I-0109 — Return commits the keyboard selection.
        //
        // Arrow keys move the List's highlight without navigating, which the writer
        // explicitly wants kept ("it allows me to rapidly move to another scene").
        // But Return did nothing at all, so a scene reached by keyboard could not be
        // opened without going back to the mouse. Navigation was bound solely to
        // `onTapGesture` (see `sceneRow`), so no key path reached it.
        //
        // ⚠️ **Both platforms are now selection-driven** (I-0132), so moving the highlight
        // IS selecting, and selection drives the manuscript. The Return handler is kept
        // only to give the *keyboard* an explicit commit that also transfers focus — a
        // writer who arrowed to a scene should be able to press Return and start typing.
        // It no longer guards on `selection == nil`, which is never true now and would
        // have made this dead code.
        #if os(macOS)
        .onKeyPress(.return) {
            guard let sceneID = selection?.wrappedValue ?? highlightedRowID
                    .flatMap({ $0.hasPrefix("scene-") ? String($0.dropFirst("scene-".count)) : nil })
            else { return .ignored }
            navigate(to: sceneID)
            return .handled
        }
        #endif
        .onChange(of: loader.viewportSceneID) { _, sceneID in
            // Sync highlight to viewport as the manuscript scrolls.
            if let selection {
                // iOS: reflect the scrolled-to scene in the selection (and thus the highlight)
                // without re-triggering a scroll. Only update when it actually differs.
                if let sceneID, selection.wrappedValue != sceneID {
                    selection.wrappedValue = sceneID
                }
            } else {
                // macOS: highlight only — must NOT navigate. Suppress the selection
                // observer below for this programmatic write, or scrolling the manuscript
                // would bounce back through navigate() and fight the writer's scroll.
                highlightedRowID = sceneID.map { "scene-\($0)" }
            }
        }
        // MARK: Rename sheet
        .sheet(item: $renameTarget) { target in
            RenameSheet(
                target: target,
                onSave: { newTitle in
                    renameTarget = nil
                    performRename(target: target, newTitle: newTitle)
                },
                onCancel: {
                    renameTarget = nil
                }
            )
        }
        // MARK: Delete scene confirmation
        .confirmationDialog(
            "Delete Scene?",
            isPresented: Binding(
                get: { deleteSceneTarget != nil },
                set: { if !$0 { deleteSceneTarget = nil } }
            ),
            titleVisibility: .visible
        ) {
            if let entry = deleteSceneTarget {
                Button("Delete \u{201C}\(truncated(entry.title))\u{201D}", role: .destructive) {
                    performDeleteScene(entry: entry)
                    deleteSceneTarget = nil
                }
                Button("Cancel", role: .cancel) {
                    deleteSceneTarget = nil
                }
            }
        } message: {
            Text("This scene will be permanently deleted and cannot be recovered.")
        }
        // MARK: Delete chapter confirmation
        .confirmationDialog(
            "Delete Chapter?",
            isPresented: Binding(
                get: { deleteChapterTarget != nil },
                set: { if !$0 { deleteChapterTarget = nil } }
            ),
            titleVisibility: .visible
        ) {
            if let group = deleteChapterTarget {
                Button("Delete \u{201C}\(truncated(group.chapterTitle))\u{201D} and all its scenes", role: .destructive) {
                    performDeleteChapter(group: group)
                    deleteChapterTarget = nil
                }
                Button("Cancel", role: .cancel) {
                    deleteChapterTarget = nil
                }
            }
        } message: {
            if let group = deleteChapterTarget {
                let count = group.scenes.count
                let noun = count == 1 ? "scene" : "scenes"
                Text("\u{201C}\(truncated(group.chapterTitle))\u{201D} contains \(count) \(noun). All scenes will be permanently deleted and cannot be recovered.")
            }
        }
        // MARK: Error alert
        .alert("Error", isPresented: Binding(
            get: { alertError != nil },
            set: { if !$0 { alertError = nil } }
        )) {
            Button("OK") { alertError = nil }
        } message: {
            Text(alertError ?? "")
        }
    }

    // MARK: — Scene row

    @ViewBuilder
    private func sceneRow(for entry: SceneEntry, in group: ChapterGroup) -> some View {
        let isActive = entry.sceneID == loader.viewportSceneID
        NavigatorSceneRow(title: entry.title, isActive: isActive)
            .tag("scene-\(entry.sceneID)")
            .listRowBackground(isActive ? Color.accentColor.opacity(0.12) : Color.clear)
            // ⚠️ **No tap gesture on either platform (I-0132, 2026-08-18).** Navigation is
            // driven by the List's `selection` binding, which the `NSTableView` behind
            // `List` updates on every click. Three earlier attempts tried to win a race
            // against that table — synchronous `makeFirstResponder`, then a deferred one,
            // then `simultaneousGesture` — and each was a smaller race rather than a fix.
            // The selection was always correct; only the parallel state kept beside it
            // was unreliable. So there is no longer any parallel state.
            .contextMenu {
                Button("Rename") { renameTarget = .scene(entry) }
                Divider()
                Button("Delete", role: .destructive) { deleteSceneTarget = entry }
            }
            .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                Button("Delete", role: .destructive) { deleteSceneTarget = entry }
            }
    }

    // MARK: — Chapter header row

    @ViewBuilder
    private func chapterHeaderRow(for group: ChapterGroup) -> some View {
        Text(group.chapterTitle)
            .font(.subheadline.weight(.semibold))
            .foregroundStyle(.primary)
            .padding(.top, 6)
            .tag("header-\(group.chapterID)")
            .listRowBackground(Color.clear)
            .contextMenu {
                Button("Rename") {
                    renameTarget = .chapter(group)
                }
                Divider()
                Button("Delete", role: .destructive) {
                    deleteChapterTarget = group
                }
            }
    }

    // MARK: — Navigation

    // Go to a scene. **Writes the selection** — the single source of truth — and lets the
    // owning view's `onChange` do the rest: caret to the scene's first character (§3),
    // scroll, and focus transfer. Callers must not also poke the manuscript directly, or
    // the two paths can disagree.
    //
    // Falls back to `onNavigate` when no selection binding was supplied (a preview or a
    // host that drives navigation itself).
    private func navigate(to sceneID: String) {
        if let selection {
            // Re-selecting the same row is a no-op by design: `onChange` won't fire, and a
            // click on the scene you are already in should not re-scroll or move the caret.
            selection.wrappedValue = sceneID
            // Focus still transfers, so clicking the current scene is a usable way to get
            // the keyboard back into the manuscript.
            loader.takeFocus()
        } else {
            onNavigate(sceneID)
            loader.takeFocus()
        }
    }

    // MARK: — Rename

    private func performRename(target: RenameTarget, newTitle: String) {
        guard let projectRootPath = session.projectRootPath else { return }
        Task { @MainActor in
            do {
                switch target {
                case .scene(let entry):
                    _ = try env.engine.renameScene(
                        projectRootPath: projectRootPath,
                        metadataPath: entry.metadataPath,
                        newTitle: newTitle
                    )
                    loader.updateSceneTitle(newTitle, forMetadataPath: entry.metadataPath)
                case .chapter(let group):
                    _ = try env.engine.renameChapter(
                        projectRootPath: projectRootPath,
                        metadataPath: group.chapterMetadataPath,
                        newTitle: newTitle
                    )
                    loader.updateChapterTitle(newTitle, forChapterMetadataPath: group.chapterMetadataPath)
                }
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Delete

    private func performDeleteScene(entry: SceneEntry) {
        guard let projectRootPath = session.projectRootPath else { return }
        Task { @MainActor in
            do {
                // Record a structural barrier BEFORE the delete so undo can't walk
                // back into the deleted scene, and — with I-0066 — so the scene's
                // now-orphaned history is pruned cleanly on the next open instead of
                // replaying a diff against a scene that no longer exists.
                session.historyCapture?.recordBarrier(kind: "sceneDelete",
                                                      note: "Can't undo past deleting a scene")
                _ = try env.engine.deleteScene(
                    projectRootPath: projectRootPath,
                    sceneID: entry.sceneID
                )
                if let nextSceneID = loader.removeScene(sceneID: entry.sceneID) {
                    // Through `navigate` so the selection stays the source of truth —
                    // calling `onNavigate` directly left the navigator selected on a
                    // scene that no longer exists.
                    navigate(to: nextSceneID)
                }
                // Transfer first-responder directly in AppKit.
                onTakeFocus()
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    private func performDeleteChapter(group: ChapterGroup) {
        guard let projectRootPath = session.projectRootPath else { return }
        let wasCurrentChapter = loader.currentSegment.map { seg in group.scenes.contains { $0.sceneID == seg.sceneID } } ?? false
        Task { @MainActor in
            do {
                // Structural barrier before the delete (I-0066): a deleted chapter
                // takes all its scenes' history out of reach; the barrier stops undo
                // and the load-time prune cleans up the orphaned diffs next open.
                session.historyCapture?.recordBarrier(kind: "sceneDelete",
                                                      note: "Can't undo past deleting a chapter")
                _ = try env.engine.deleteChapter(
                    projectRootPath: projectRootPath,
                    chapterID: group.chapterID
                )
                if let nextSceneID = loader.removeChapter(chapterID: group.chapterID) {
                    navigate(to: nextSceneID)
                }
                if wasCurrentChapter {
                    onTakeFocus()
                }
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — onMove handler

    private func performMove(from source: IndexSet, to destination: Int) {
        guard let sourceIdx = source.first else { return }
        let rows = flatRows

        guard sourceIdx < rows.count else { return }
        let movedRow = rows[sourceIdx]

        // SwiftUI destination is the insertion point in the original array (source still present).
        // The row immediately before the landing spot is rows[destination - 1].
        let predecessorIdx = destination - 1

        switch movedRow {
        case .scene(let entry, let sourceGroup):
            var afterSceneID = ""
            var targetGroup: ChapterGroup? = nil

            if predecessorIdx < 0 {
                // Dropped to very top of the list — first scene of the first chapter.
                targetGroup = rows.compactMap {
                    if case .chapterHeader(let g) = $0 { return g } else { return nil }
                }.first
                afterSceneID = ""
            } else {
                let prevRow = rows[min(predecessorIdx, rows.count - 1)]
                switch prevRow {
                case .chapterHeader(let g):
                    // Landing right after a chapter header = first scene of that chapter.
                    targetGroup = g
                    afterSceneID = ""
                case .scene(let prevEntry, let prevGroup):
                    targetGroup = prevGroup
                    afterSceneID = prevEntry.sceneID
                }
            }

            guard let tg = targetGroup else { return }

            performReorderScene(
                sceneID: entry.sceneID,
                sourceChapterID: sourceGroup.chapterID,
                targetGroup: tg,
                afterSceneID: afterSceneID
            )

        case .chapterHeader(let group):
            var afterChapterID = ""

            if predecessorIdx < 0 {
                afterChapterID = ""
            } else {
                let prevRow = rows[min(predecessorIdx, rows.count - 1)]
                switch prevRow {
                case .chapterHeader(let g):
                    afterChapterID = g.chapterID
                case .scene(_, let prevGroup):
                    afterChapterID = prevGroup.chapterID
                }
            }

            performReorderChapter(chapterID: group.chapterID, afterChapterID: afterChapterID)
        }
    }

    // MARK: — Reorder scene

    private func performReorderScene(
        sceneID: String,
        sourceChapterID: String,
        targetGroup: ChapterGroup,
        afterSceneID: String
    ) {
        guard let projectRootPath = session.projectRootPath else { return }
        loader.reorderScene(
            sceneID: sceneID,
            targetChapterID: targetGroup.chapterID,
            afterSceneID: afterSceneID,
            targetChapterMetadataPath: targetGroup.chapterMetadataPath,
            targetChapterTitle: targetGroup.chapterTitle
        )
        Task { @MainActor in
            do {
                _ = try env.engine.reorderScene(
                    projectRootPath: projectRootPath,
                    sceneID: sceneID,
                    sourceChapterID: sourceChapterID,
                    targetChapterID: targetGroup.chapterID,
                    afterSceneID: afterSceneID
                )
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Reorder chapter

    private func performReorderChapter(chapterID: String, afterChapterID: String) {
        guard let projectRootPath = session.projectRootPath else { return }
        loader.reorderChapter(chapterID: chapterID, afterChapterID: afterChapterID)
        Task { @MainActor in
            do {
                _ = try env.engine.reorderChapter(
                    projectRootPath: projectRootPath,
                    chapterID: chapterID,
                    afterChapterID: afterChapterID
                )
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Search (T-0569)

    private var searchField: some View {
        HStack(spacing: 4) {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(.secondary)
            TextField("Search", text: $searchText)
                .textFieldStyle(.plain)
            if !searchText.isEmpty {
                Button {
                    searchText = ""
                } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.borderless)
                .help("Clear Search")
            }
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
    }

    /// The query as matched — surrounding whitespace is not part of it.
    private var searchQuery: String {
        searchText.trimmingCharacters(in: .whitespaces)
    }

    /// IDs of the scenes whose text contains `query`, or nil if the search was cancelled.
    ///
    /// ✅ `localizedStandardContains` — case- and diacritic-insensitive, locale-aware: the
    /// platform's own user-facing search semantics. ⛔ NOT `String.range(of:options:)`,
    /// measured ~10× slower (~260 ms vs ~24 ms on 4.2 MB, 2026-10-01).
    @concurrent
    private nonisolated static func scenesMatching(_ query: String,
                                                    in texts: [SceneText]) async -> Set<String>? {
        var ids: Set<String> = []
        for (i, scene) in texts.enumerated() {
            if i % 64 == 0, Task.isCancelled { return nil }
            if scene.text.localizedStandardContains(query) { ids.insert(scene.sceneID) }
        }
        return Task.isCancelled ? nil : ids
    }

    struct SearchKey: Equatable {
        let query: String
        let sceneCount: Int
    }

    struct SceneText: Sendable {
        let sceneID: String
        let text: String
    }

    struct BodyMatches {
        let query: String
        let sceneIDs: Set<String>
    }

    // MARK: — Helpers

    private func truncated(_ title: String, limit: Int = 30) -> String {
        guard title.count > limit else { return title }
        return title.prefix(limit) + "\u{2026}"
    }

    // MARK: — Data model

    struct SceneEntry: Identifiable {
        var id: String { sceneID }
        let sceneID: String
        let title: String
        let metadataPath: String
    }

    struct ChapterGroup: Identifiable {
        var id: String { chapterID }
        let chapterID: String
        let chapterTitle: String
        let chapterMetadataPath: String
        var scenes: [SceneEntry]
    }

    // Flat row type for the single ForEach that enables onMove across chapter boundaries.
    enum FlatRow {
        case chapterHeader(ChapterGroup)
        case scene(SceneEntry, ChapterGroup)

        var rowID: String {
            switch self {
            case .chapterHeader(let g): return "header-\(g.chapterID)"
            case .scene(let e, _):      return "scene-\(e.sceneID)"
            }
        }
    }

    enum RenameTarget: Identifiable {
        case scene(SceneEntry)
        case chapter(ChapterGroup)

        var id: String {
            switch self {
            case .scene(let e):   return "scene-\(e.sceneID)"
            case .chapter(let g): return "chapter-\(g.chapterID)"
            }
        }

        var currentTitle: String {
            switch self {
            case .scene(let e):   return e.title
            case .chapter(let g): return g.chapterTitle
            }
        }

        var promptLabel: String {
            switch self {
            case .scene:   return "Scene Name"
            case .chapter: return "Chapter Name"
            }
        }
    }

    // Flat array of rows used by the List. Chapter header rows interleaved with scene rows.
    //
    // T-0569 — while searching, a chapter whose TITLE matches keeps all its scenes; otherwise
    // a scene is kept when its displayed title or its body matches, under its chapter header.
    // Titles are matched here (cheap); bodies come from the off-main scan (`bodyMatches`).
    private var flatRows: [FlatRow] {
        let query = searchQuery
        let bodyIDs = bodyMatches?.query == query ? bodyMatches?.sceneIDs : nil
        var rows: [FlatRow] = []
        for group in chapterGroups {
            let scenes: [SceneEntry]
            if query.isEmpty || group.chapterTitle.localizedStandardContains(query) {
                scenes = group.scenes
            } else {
                scenes = group.scenes.filter {
                    $0.title.localizedStandardContains(query) || bodyIDs?.contains($0.sceneID) == true
                }
                if scenes.isEmpty { continue }
            }
            rows.append(.chapterHeader(group))
            for entry in scenes {
                rows.append(.scene(entry, group))
            }
        }
        return rows
    }

    private var chapterGroups: [ChapterGroup] {
        var groups: [ChapterGroup] = []
        var chapterOrdinal = 0
        var sceneOrdinal = 0

        for info in loader.allScenes {
            sceneOrdinal += 1

            let title: String
            if !info.title.trimmingCharacters(in: .whitespaces).isEmpty {
                title = info.title
            } else {
                let liveText = loader.liveTitles[info.sceneID] ?? ""
                title = liveText.trimmingCharacters(in: .whitespaces).isEmpty
                    ? "Scene \(sceneOrdinal)"
                    : liveText
            }

            let entry = SceneEntry(
                sceneID: info.sceneID,
                title: title,
                metadataPath: info.metadataPath
            )

            if let lastIdx = groups.indices.last,
               groups[lastIdx].chapterID == info.chapterID {
                groups[lastIdx].scenes.append(entry)
            } else {
                chapterOrdinal += 1
                let chapterTitle = info.chapterTitle.trimmingCharacters(in: .whitespaces).isEmpty
                    ? "Chapter \(chapterOrdinal)"
                    : info.chapterTitle
                groups.append(ChapterGroup(
                    chapterID: info.chapterID,
                    chapterTitle: chapterTitle,
                    chapterMetadataPath: info.chapterMetadataPath,
                    scenes: [entry]
                ))
            }
        }
        return groups
    }
}

// MARK: — Scene row view

private struct NavigatorSceneRow: View {
    let title: String
    let isActive: Bool

    var body: some View {
        Text(title)
            .font(.callout)
            .lineLimit(1)
            .truncationMode(.tail)
            .foregroundStyle(isActive ? .primary : .secondary)
            .padding(.vertical, 2)
            .padding(.leading, 8)
    }
}

// MARK: — Rename sheet

private struct RenameSheet: View {
    let target: SceneNavigatorView.RenameTarget
    let onSave: (String) -> Void
    let onCancel: () -> Void

    @State private var text: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        NavigationStack {
            Form {
                TextField(target.promptLabel, text: $text)
                    .focused($focused)
                    .onSubmit { onSave(text) }
            }
            .navigationTitle("Rename")
            #if os(macOS)
            .padding()
            #endif
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") { onSave(text) }
                }
            }
        }
        .onAppear {
            text = target.currentTitle
            focused = true
        }
        #if os(macOS)
        .frame(minWidth: 320, minHeight: 120)
        #endif
    }
}
