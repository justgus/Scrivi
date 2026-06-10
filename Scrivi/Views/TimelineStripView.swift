import SwiftUI

struct TimelineStripView: View {

    var body: some View {
        Divider()
        VStack(spacing: 0) {
            Text("Timeline")
                .font(.caption)
                .foregroundStyle(.secondary)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding(.horizontal, 12)
                .padding(.vertical, 4)

            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 10) {
                    TimelineMarkerView(title: "Event 1", color: .accentColor)
                    TimelineMarkerView(title: "Event 2", color: .secondary)
                    TimelineMarkerView(title: "Event 3", color: .accentColor)
                    TimelineMarkerView(title: "Event 4", color: .secondary)
                    TimelineMarkerView(title: "Event 5", color: .accentColor)
                    TimelineMarkerView(title: "Event 6", color: .secondary)
                    TimelineMarkerView(title: "Event 7", color: .accentColor)
                }
                .padding(.horizontal, 12)
            }
            .frame(maxWidth: .infinity)
        }
        .frame(maxWidth: .infinity)
        .frame(height: 80)
    }
}

// MARK: — Stub event marker

private struct TimelineMarkerView: View {
    let title: String
    let color: Color

    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 6)
                .fill(color.opacity(0.2))
            Text(title)
                .font(.caption2)
                .foregroundStyle(color)
        }
        .frame(width: 80, height: 40)
    }
}
