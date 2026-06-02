---
name: T-0078
title: "ManuscriptTextView — NSViewRepresentable wrapping NSTextView; displays concatenated scene segments separated by divider views"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Created `ManuscriptTextView.swift` in `Sources/ScriviApp/`. `NSViewRepresentable` wrapping a single `ManuscriptNSTextView` (NSTextView subclass). Scene boundaries tracked as `[NSRange]` in `Coordinator.sceneBoundaries`. `updateNSView` rebuilds `NSTextStorage` from segments when the segment ID list changes. `rebuildStorage` concatenates scene text with divider attachments between segments.
