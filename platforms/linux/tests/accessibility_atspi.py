#!/usr/bin/env python3
"""accessibility_atspi.py — EP-050 S4 (SP-203, T-0659): the manuscript read OVER AT-SPI, the way Orca reads it.

Run by accessibility_atspi.sh while scrivi_linux_accessibility_smoke holds its fixture on screen. Uses the same calls
Orca makes: GetText, GetStringAtOffset (line), SetCaretOffset / GetCaretOffset, and the object:text-caret-moved and
text-changed:insert events.
Prints `atspi-ok` when every check passes.
"""
import sys

import gi

gi.require_version("Atspi", "2.0")
from gi.repository import Atspi, GLib  # noqa: E402

PAGE = ("Chapter One\n\nMr. Smith said bold and soft here.\n\nThe claim stated plainly\n\n- a list item"
        "\nScene break\nSecond scene.\n\nChapter Two\n\nThe third scene, 1. not a list.")
APP = "scrivi_linux_accessibility_smoke"
failures = []


def check(ok, what):
    if not ok:
        failures.append(what)
        print("FAIL:", what, file=sys.stderr)


def find_text(node, depth=0):
    if node is None or depth > 30:
        return None
    try:
        text = node.get_text_iface()
    except GLib.Error:
        text = None
    if text is not None and node.get_role() in (Atspi.Role.ENTRY, Atspi.Role.TEXT, Atspi.Role.DOCUMENT_TEXT) \
            and Atspi.Text.get_character_count(node) > 0:
        return node
    for i in range(node.get_child_count()):
        found = find_text(node.get_child_at_index(i), depth + 1)
        if found is not None:
            return found
    return None


def main():
    Atspi.init()
    desktop = Atspi.get_desktop(0)
    app = next((desktop.get_child_at_index(i) for i in range(desktop.get_child_count())
                if desktop.get_child_at_index(i).get_name() == APP), None)
    if app is None:
        print("FAIL: the smoke's application is not on the AT-SPI bus", file=sys.stderr)
        return 1
    editor = find_text(app)
    if editor is None:
        print("FAIL: no text object in the application", file=sys.stderr)
        return 1

    n = Atspi.Text.get_character_count(editor)
    text = Atspi.Text.get_text(editor, 0, n)
    check(text == PAGE, f"GetText is the page at rest\n   got:      {text!r}\n   expected: {PAGE!r}")
    check(n == len(PAGE), f"character count {n} = the page's {len(PAGE)}")

    smith = PAGE.index("Smith")
    line = Atspi.Text.get_string_at_offset(editor, smith, Atspi.TextGranularity.LINE)
    check(line.content.rstrip("\n") == "Mr. Smith said bold and soft here.", f"the line at 'Smith': {line.content!r}")
    brk = PAGE.index("Scene break")
    line = Atspi.Text.get_string_at_offset(editor, brk + 2, Atspi.TextGranularity.LINE)
    check(line.content.rstrip("\n") == "Scene break", f"the divider's line: {line.content!r}")

    # The caret, over the bus, and the event Orca follows.
    moved = []

    def on_moved(event):
        if event.source == editor:
            moved.append(event.detail1)

    listener = Atspi.EventListener.new(on_moved)
    listener.register("object:text-caret-moved")
    soft = PAGE.index("soft")
    Atspi.Text.set_caret_offset(editor, soft)
    GLib.timeout_add(1500, lambda: (Atspi.event_quit(), False)[1])
    Atspi.event_main()
    check(Atspi.Text.get_caret_offset(editor) == soft,
          f"the caret set at 'soft' ({soft}) reads back at {Atspi.Text.get_caret_offset(editor)}")
    check(moved and moved[-1] == soft, f"text-caret-moved carries the page position {soft}: {moved}")

    # An edit, typed into this display's own X server (never the desktop): its event carries the page position.
    inserted = []

    def on_insert(event):
        if event.source == editor:
            inserted.append((event.detail1, event.any_data))

    typing = Atspi.EventListener.new(on_insert)
    typing.register("object:text-changed:insert")
    Atspi.generate_keyboard_event(0, "q", Atspi.KeySynthType.STRING)
    GLib.timeout_add(1500, lambda: (Atspi.event_quit(), False)[1])
    Atspi.event_main()
    check(inserted and inserted[-1][0] == soft, f"text-changed:insert at the page position {soft}: {inserted}")
    check("and qsoft here." in Atspi.Text.get_text(editor, 0, -1), "the typed character is on the page")

    if failures:
        return 1
    print("atspi-ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
