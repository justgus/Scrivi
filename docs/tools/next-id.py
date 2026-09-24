#!/usr/bin/env python3
"""Allocate the next tracking ID for an Epic, Sprint, Task or Issue.

⚠️ WHY THIS EXISTS. The "next available ID" figure used to be restated inside
`Sprint-active.md` and echoed across the tracking documents. That is a number the
reader does not need and the writer kept getting wrong: the Task index claimed
`T-0502` while `T-0503`–`T-0548` had all been issued, and `Sprint-Documentation.md`
still carried `SP-107` and `SP-119` in its prose. ✅ The figure now lives in exactly
one machine-readable place — `next-ids.json` — and NOT in any tracking document.

USAGE
  python3 docs/tools/next-id.py --peek              # show all, change nothing
  python3 docs/tools/next-id.py task                # allocate ONE task id
  python3 docs/tools/next-id.py task --count 2      # allocate two, contiguous
  python3 docs/tools/next-id.py sprint --dry-run    # show what WOULD be taken

⚠️ Allocation WRITES: the stored value is incremented so the same ID is never
handed out twice. Use --peek or --dry-run to look without consuming.
"""

import argparse
import json
import pathlib
import sys

STORE = pathlib.Path(__file__).with_name("next-ids.json")
KINDS = ("epic", "sprint", "task", "issue")


def load():
    try:
        return json.loads(STORE.read_text(encoding="utf-8"))
    except FileNotFoundError:
        sys.exit(f"error: {STORE} not found")
    except json.JSONDecodeError as exc:
        # ⚠️ Never guess past a corrupt store — a wrong guess reissues a live ID.
        sys.exit(f"error: {STORE} is not valid JSON ({exc})")


def save(data):
    STORE.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n",
                     encoding="utf-8")


def render(data, kind, number):
    return data["format"][kind] % number


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("kind", nargs="?", choices=KINDS,
                    help="which counter to allocate from")
    ap.add_argument("--count", type=int, default=1,
                    help="allocate this many contiguous IDs (default 1)")
    ap.add_argument("--peek", action="store_true",
                    help="print every next-available ID and exit without writing")
    ap.add_argument("--dry-run", action="store_true",
                    help="show what would be allocated, but do not consume it")
    args = ap.parse_args()

    data = load()

    if args.peek or args.kind is None:
        width = max(len(k) for k in KINDS)
        for k in KINDS:
            print(f"{k:<{width}}  {render(data, k, data['next'][k])}")
        if args.kind is None and not args.peek:
            print("\n(nothing allocated — name a kind to allocate, "
                  "e.g. `next-id.py task`)", file=sys.stderr)
        return

    if args.count < 1:
        sys.exit("error: --count must be at least 1")

    start = data["next"][args.kind]
    ids = [render(data, args.kind, start + i) for i in range(args.count)]

    if args.dry_run:
        print(" ".join(ids))
        print("(--dry-run: nothing consumed)", file=sys.stderr)
        return

    data["next"][args.kind] = start + args.count
    save(data)
    print(" ".join(ids))


if __name__ == "__main__":
    main()
