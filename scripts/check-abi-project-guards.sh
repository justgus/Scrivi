#!/usr/bin/env bash
# SP-173 / I-0285 — every C ABI endpoint that takes a project runs under that project's lock (D1) and reports its
# revision (D2). Design: docs/Scrivi_Core_Concurrency_Design_v0_1.md.
#
# ⚠️ WHY THIS EXISTS. The read/write classification is ONE WORD at each endpoint (`SCRIVI_PROJECT_READ` /
# `SCRIVI_PROJECT_WRITE`), never a list kept elsewhere (CLAUDE.md: lists must be derived, never restated). This check is
# what keeps a NEW endpoint from shipping unlocked: it fails if an endpoint with a `projectRoot…` parameter does not open
# with a guard on that same parameter.
#
# Run: scripts/check-abi-project-guards.sh     (exit 1 on an unguarded project endpoint). Registered in ctest.
set -uo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
import re, sys
s = open('ScriviCore/src/public_api/scrivi_c_api.cpp').read()
bad, n = [], 0
for m in re.finditer(r'^const char\* (scrivi_\w+)\(([^)]*)\)\s*\{\s*\n\s*([^\n]*)', s, re.M):
    name, params, first = m.groups()
    pm = re.search(r'const char\*\s*(projectRoot\w*)', params)
    if not pm:
        continue
    n += 1
    if not re.fullmatch(r'SCRIVI_PROJECT_(READ|WRITE)\(' + pm.group(1) + r'\);', first.strip()):
        bad.append(f'{name}: first statement is `{first.strip()}`')
if bad:
    print('⛔ Project endpoints without SCRIVI_PROJECT_READ/WRITE as their first statement (SP-173 D1):')
    print('\n'.join('   ' + b for b in bad))
    sys.exit(1)
print(f'✅ All {n} project endpoints open with a project guard.')
PY
