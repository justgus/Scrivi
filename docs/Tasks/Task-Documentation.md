# Task — Index

The complete register of every Task ID in the project.

> **Rebuilt 2026-08-15 from the filesystem and the Sprint records.** The previous index carried **239**
> of the **411** Task IDs that actually exist — 172 were missing, almost all of them completed work whose
> Sprint had closed. Statuses are taken from each Task's **closed Sprint record** where one exists, since
> `Task-backlog.md` rows were found to go stale after their Sprint closed.

## Organization

| File | Holds |
| ---- | ----- |
| [`Task-active.md`](Task-active.md) | Tasks worked by the **active Sprint** (Tasks only — Sprint documentation lives in [`../Sprints/`](../Sprints/)) |
| [`Task-backlog.md`](Task-backlog.md) | Planned Tasks not yet in a Sprint |
| [`Task-unverified.md`](Task-unverified.md) | Implemented, awaiting user verification |
| [`Verified/`](Verified/) | Archived Tasks, user-verified |
| [`Closed/`](Closed/) | Archived Tasks closed without verification |

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

---

## Statistics

- **Total Task IDs:** **411** — T-0001 … T-0412 (T-0278 never assigned; T-0412 unassigned)
- **✅ Verified:** 369
- **🔵 Backlog:** 24
- **🟠 Implemented - Not Verified:** 9
- **⚪ Superseded:** 3
- **⚪ Closed:** 2
- **⛔️ Removed as OBE:** 2
- **⚪ Never assigned:** 1
- **⚪ Unassigned:** 1
- **⚠️ Verified but UNFILED:** **180** — see the section below

---

## ⚠️ Verified but unfiled

These **180** Tasks were completed and verified inside a Sprint that has since closed, but no
`Verified/Task-verified-XXXX.md` archive file was ever written for them. **Their record of truth is the
Sprint archive named in each row** — this is unfiled work, not missing work.

Per the user's ruling (2026-08-15) they are indexed as-is and flagged, rather than back-filled with
archive files reconstructed after the fact from secondary sources.

| Task | Sprint | Epic | Record of truth |
| ---- | ------ | ---- | --------------- |
| T-0090 | SP-026 | — | [`Sprint-SP-026.md`](../Sprints/Closed/Sprint-SP-026.md) |
| T-0091 | SP-026 | — | [`Sprint-SP-026.md`](../Sprints/Closed/Sprint-SP-026.md) |
| T-0092 | SP-026 | — | [`Sprint-SP-026.md`](../Sprints/Closed/Sprint-SP-026.md) |
| T-0093 | SP-026 | — | [`Sprint-SP-026.md`](../Sprints/Closed/Sprint-SP-026.md) |
| T-0094 | SP-026 | — | [`Sprint-SP-026.md`](../Sprints/Closed/Sprint-SP-026.md) |
| T-0097 | SP-027 | — | [`Sprint-SP-027.md`](../Sprints/Closed/Sprint-SP-027.md) |
| T-0098 | SP-027 | — | [`Sprint-SP-027.md`](../Sprints/Closed/Sprint-SP-027.md) |
| T-0099 | SP-028 | — | [`Sprint-SP-028.md`](../Sprints/Closed/Sprint-SP-028.md) |
| T-0100 | SP-028 | — | [`Sprint-SP-028.md`](../Sprints/Closed/Sprint-SP-028.md) |
| T-0101 | SP-029 | — | [`Sprint-SP-029.md`](../Sprints/Closed/Sprint-SP-029.md) |
| T-0102 | SP-029 | — | [`Sprint-SP-029.md`](../Sprints/Closed/Sprint-SP-029.md) |
| T-0103 | SP-030 | — | [`Sprint-SP-030.md`](../Sprints/Closed/Sprint-SP-030.md) |
| T-0104 | SP-030 | — | [`Sprint-SP-030.md`](../Sprints/Closed/Sprint-SP-030.md) |
| T-0105 | SP-030 | — | [`Sprint-SP-030.md`](../Sprints/Closed/Sprint-SP-030.md) |
| T-0109 | SP-032 | EP-010 | [`Sprint-SP-032.md`](../Sprints/Closed/Sprint-SP-032.md) |
| T-0110 | SP-032 | EP-010 | [`Sprint-SP-032.md`](../Sprints/Closed/Sprint-SP-032.md) |
| T-0111 | SP-032 | EP-010 | [`Sprint-SP-032.md`](../Sprints/Closed/Sprint-SP-032.md) |
| T-0156 | SP-041 | EP-016 | [`Sprint-SP-041.md`](../Sprints/Closed/Sprint-SP-041.md) |
| T-0157 | SP-041 | EP-016 | [`Sprint-SP-041.md`](../Sprints/Closed/Sprint-SP-041.md) |
| T-0158 | SP-041 | EP-016 | [`Sprint-SP-041.md`](../Sprints/Closed/Sprint-SP-041.md) |
| T-0159 | SP-041 | EP-016 | [`Sprint-SP-041.md`](../Sprints/Closed/Sprint-SP-041.md) |
| T-0160 | SP-041 | EP-016 | [`Sprint-SP-041.md`](../Sprints/Closed/Sprint-SP-041.md) |
| T-0161 | SP-042 | EP-016 | [`Sprint-SP-042.md`](../Sprints/Closed/Sprint-SP-042.md) |
| T-0162 | SP-042 | EP-016 | [`Sprint-SP-042.md`](../Sprints/Closed/Sprint-SP-042.md) |
| T-0163 | SP-042 | EP-016 | [`Sprint-SP-042.md`](../Sprints/Closed/Sprint-SP-042.md) |
| T-0164 | SP-042 | EP-016 | [`Sprint-SP-042.md`](../Sprints/Closed/Sprint-SP-042.md) |
| T-0165 | SP-042 | EP-016 | [`Sprint-SP-042.md`](../Sprints/Closed/Sprint-SP-042.md) |
| T-0185 | SP-046 | EP-017 | [`Sprint-SP-046.md`](../Sprints/Closed/Sprint-SP-046.md) |
| T-0186 | SP-046 | EP-017 | [`Sprint-SP-046.md`](../Sprints/Closed/Sprint-SP-046.md) |
| T-0187 | SP-046 | EP-017 | [`Sprint-SP-046.md`](../Sprints/Closed/Sprint-SP-046.md) |
| T-0188 | SP-046 | EP-017 | [`Sprint-SP-046.md`](../Sprints/Closed/Sprint-SP-046.md) |
| T-0191 | SP-048 | EP-018 | [`Sprint-SP-048.md`](../Sprints/Closed/Sprint-SP-048.md) |
| T-0199 | SP-051 | EP-019 | [`Sprint-SP-051.md`](../Sprints/Closed/Sprint-SP-051.md) |
| T-0213 | SP-056 | EP-019 | [`Sprint-SP-056.md`](../Sprints/Closed/Sprint-SP-056.md) |
| T-0214 | SP-056 | EP-019 | [`Sprint-SP-056.md`](../Sprints/Closed/Sprint-SP-056.md) |
| T-0217 | SP-094 | EP-019 | [`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md) |
| T-0234 | SP-061 | EP-021 | [`Sprint-SP-061.md`](../Sprints/Closed/Sprint-SP-061.md) |
| T-0235 | SP-061 | EP-022 | [`Sprint-SP-061.md`](../Sprints/Closed/Sprint-SP-061.md) |
| T-0236 | SP-061 | EP-023 | [`Sprint-SP-061.md`](../Sprints/Closed/Sprint-SP-061.md) |
| T-0237 | SP-061 | EP-022 | [`Sprint-SP-061.md`](../Sprints/Closed/Sprint-SP-061.md) |
| T-0238 | SP-062 | EP-022 | [`Sprint-SP-062.md`](../Sprints/Closed/Sprint-SP-062.md) |
| T-0239 | SP-062 | EP-022 | [`Sprint-SP-062.md`](../Sprints/Closed/Sprint-SP-062.md) |
| T-0240 | SP-062 | EP-022 | [`Sprint-SP-062.md`](../Sprints/Closed/Sprint-SP-062.md) |
| T-0241 | SP-062 | EP-022 | [`Sprint-SP-062.md`](../Sprints/Closed/Sprint-SP-062.md) |
| T-0242 | SP-062 | EP-022 | [`Sprint-SP-062.md`](../Sprints/Closed/Sprint-SP-062.md) |
| T-0243 | SP-063 | EP-022 | [`Sprint-SP-063.md`](../Sprints/Closed/Sprint-SP-063.md) |
| T-0244 | SP-063 | EP-022 | [`Sprint-SP-063.md`](../Sprints/Closed/Sprint-SP-063.md) |
| T-0245 | SP-063 | EP-022 | [`Sprint-SP-063.md`](../Sprints/Closed/Sprint-SP-063.md) |
| T-0258 | SP-067 | EP-023 | [`Sprint-SP-067.md`](../Sprints/Closed/Sprint-SP-067.md) |
| T-0259 | SP-067 | EP-023 | [`Sprint-SP-067.md`](../Sprints/Closed/Sprint-SP-067.md) |
| T-0260 | SP-067 | EP-023 | [`Sprint-SP-067.md`](../Sprints/Closed/Sprint-SP-067.md) |
| T-0262 | SP-067 | EP-023 | [`Sprint-SP-067.md`](../Sprints/Closed/Sprint-SP-067.md) |
| T-0263 | SP-067 | EP-023 | [`Sprint-SP-067.md`](../Sprints/Closed/Sprint-SP-067.md) |
| T-0264 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0265 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0266 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0267 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0268 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0269 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0270 | SP-069 | EP-027 | [`Sprint-SP-069.md`](../Sprints/Closed/Sprint-SP-069.md) |
| T-0271 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0272 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0273 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0274 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0275 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0276 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0277 | SP-070 | EP-027 | [`Sprint-SP-070.md`](../Sprints/Closed/Sprint-SP-070.md) |
| T-0279 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0280 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0281 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0282 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0283 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0284 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0285 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0286 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0287 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0288 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0289 | SP-071 | EP-027 | [`Sprint-SP-071.md`](../Sprints/Closed/Sprint-SP-071.md) |
| T-0290 | SP-072 | EP-027 | [`Sprint-SP-072.md`](../Sprints/Closed/Sprint-SP-072.md) |
| T-0291 | SP-072 | EP-027 | [`Sprint-SP-072.md`](../Sprints/Closed/Sprint-SP-072.md) |
| T-0292 | SP-072 | EP-027 | [`Sprint-SP-072.md`](../Sprints/Closed/Sprint-SP-072.md) |
| T-0293 | SP-072 | EP-027 | [`Sprint-SP-072.md`](../Sprints/Closed/Sprint-SP-072.md) |
| T-0302 | SP-075 | EP-028 | [`Sprint-SP-075.md`](../Sprints/Closed/Sprint-SP-075.md) |
| T-0303 | SP-075 | EP-028 | [`Sprint-SP-075.md`](../Sprints/Closed/Sprint-SP-075.md) |
| T-0304 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0305 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0306 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0307 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0308 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0309 | SP-076 | EP-028 | [`Sprint-SP-076.md`](../Sprints/Closed/Sprint-SP-076.md) |
| T-0310 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0311 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0312 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0313 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0314 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0315 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0316 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0317 | SP-077 | — | [`Sprint-SP-077.md`](../Sprints/Closed/Sprint-SP-077.md) |
| T-0318 | SP-078 | EP-024 | [`Sprint-SP-078.md`](../Sprints/Closed/Sprint-SP-078.md) |
| T-0319 | SP-078 | EP-024 | [`Sprint-SP-078.md`](../Sprints/Closed/Sprint-SP-078.md) |
| T-0320 | SP-078 | EP-024 | [`Sprint-SP-078.md`](../Sprints/Closed/Sprint-SP-078.md) |
| T-0321 | SP-079 | EP-025 | [`Sprint-SP-079.md`](../Sprints/Closed/Sprint-SP-079.md) |
| T-0322 | SP-079 | EP-025 | [`Sprint-SP-079.md`](../Sprints/Closed/Sprint-SP-079.md) |
| T-0323 | SP-079 | EP-025 | [`Sprint-SP-079.md`](../Sprints/Closed/Sprint-SP-079.md) |
| T-0324 | SP-079 | EP-025 | [`Sprint-SP-079.md`](../Sprints/Closed/Sprint-SP-079.md) |
| T-0325 | SP-080 | EP-025 | [`Sprint-SP-080.md`](../Sprints/Closed/Sprint-SP-080.md) |
| T-0326 | SP-080 | EP-025 | [`Sprint-SP-080.md`](../Sprints/Closed/Sprint-SP-080.md) |
| T-0327 | SP-080 | EP-025 | [`Sprint-SP-080.md`](../Sprints/Closed/Sprint-SP-080.md) |
| T-0328 | SP-080 | EP-025 | [`Sprint-SP-080.md`](../Sprints/Closed/Sprint-SP-080.md) |
| T-0329 | SP-081 | EP-025 | [`Sprint-SP-081.md`](../Sprints/Closed/Sprint-SP-081.md) |
| T-0330 | SP-081 | EP-025 | [`Sprint-SP-081.md`](../Sprints/Closed/Sprint-SP-081.md) |
| T-0331 | SP-081 | EP-025 | [`Sprint-SP-081.md`](../Sprints/Closed/Sprint-SP-081.md) |
| T-0332 | SP-081 | EP-025 | [`Sprint-SP-081.md`](../Sprints/Closed/Sprint-SP-081.md) |
| T-0333 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0334 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0335 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0336 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0337 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0338 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0339 | SP-083 | EP-025 | [`Sprint-SP-083.md`](../Sprints/Closed/Sprint-SP-083.md) |
| T-0340 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0341 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0342 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0343 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0344 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0345 | SP-082 | EP-025 | [`Sprint-SP-082.md`](../Sprints/Closed/Sprint-SP-082.md) |
| T-0346 | SP-084 | EP-025 | [`Sprint-SP-084.md`](../Sprints/Closed/Sprint-SP-084.md) |
| T-0347 | SP-084 | EP-025 | [`Sprint-SP-084.md`](../Sprints/Closed/Sprint-SP-084.md) |
| T-0348 | SP-084 | EP-025 | [`Sprint-SP-084.md`](../Sprints/Closed/Sprint-SP-084.md) |
| T-0349 | SP-084 | EP-025 | [`Sprint-SP-084.md`](../Sprints/Closed/Sprint-SP-084.md) |
| T-0350 | SP-085 | EP-029 | [`Sprint-SP-085.md`](../Sprints/Closed/Sprint-SP-085.md) |
| T-0351 | SP-086 | EP-029 | [`Sprint-SP-086.md`](../Sprints/Closed/Sprint-SP-086.md) |
| T-0352 | SP-087 | EP-029 | [`Sprint-SP-087.md`](../Sprints/Closed/Sprint-SP-087.md) |
| T-0353 | SP-088 | EP-029 | [`Sprint-SP-088.md`](../Sprints/Closed/Sprint-SP-088.md) |
| T-0354 | SP-089 | EP-029 | [`Sprint-SP-089.md`](../Sprints/Closed/Sprint-SP-089.md) |
| T-0355 | SP-089 | EP-029 | [`Sprint-SP-089.md`](../Sprints/Closed/Sprint-SP-089.md) |
| T-0356 | SP-089 | EP-029 | [`Sprint-SP-089.md`](../Sprints/Closed/Sprint-SP-089.md) |
| T-0357 | SP-089 | EP-029 | [`Sprint-SP-089.md`](../Sprints/Closed/Sprint-SP-089.md) |
| T-0359 | SP-090 | EP-030 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) |
| T-0360 | SP-090 | EP-030 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) |
| T-0361 | SP-090 | EP-030 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) |
| T-0362 | SP-090 | EP-030 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) |
| T-0363 | SP-091 | EP-030 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) |
| T-0364 | SP-091 | EP-030 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) |
| T-0366 | SP-092 | EP-030 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) |
| T-0367 | SP-092 | EP-030 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) |
| T-0368 | SP-092 | EP-030 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) |
| T-0370 | SP-095 | EP-031 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) |
| T-0371 | SP-095 | EP-031 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) |
| T-0372 | SP-095 | EP-031 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) |
| T-0373 | SP-096 | EP-031 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) |
| T-0374 | SP-096 | EP-031 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) |
| T-0375 | SP-096 | EP-031 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) |
| T-0376 | SP-096 | EP-031 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) |
| T-0377 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0378 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0379 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0380 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0381 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0382 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0383 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0384 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0385 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0386 | SP-099 | EP-031 | [`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) |
| T-0387 | SP-099 | EP-031 | [`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) |
| T-0388 | SP-099 | EP-031 | [`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) |
| T-0392 | SP-091 | EP-030 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) |
| T-0393 | SP-091 | EP-030 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) |
| T-0398 | SP-093 | EP-019 | [`Sprint-SP-093.md`](../Sprints/Closed/Sprint-SP-093.md) |
| T-0399 | SP-101 | EP-030 | [`Sprint-SP-101.md`](../Sprints/Closed/Sprint-SP-101.md) |
| T-0401 | SP-095 | EP-031 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) |
| T-0402 | SP-096 | EP-031 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) |
| T-0403 | SP-097 | EP-031 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) |
| T-0404 | SP-095 | EP-031 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) |
| T-0405 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0406 | SP-098 | EP-031 | [`Sprint-SP-098.md`](../Sprints/Closed/Sprint-SP-098.md) |
| T-0407 | SP-099 | EP-031 | [`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) |
| T-0408 | SP-099 | EP-031 | [`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) |
| T-0409 | SP-103 | EP-031 | [`Sprint-SP-103.md`](../Sprints/Closed/Sprint-SP-103.md) |
| T-0411 | SP-103 | EP-031 | [`Sprint-SP-103.md`](../Sprints/Closed/Sprint-SP-103.md) |

---

## All Tasks

| ID | Title | Sprint | Epic | Status | Archive |
| -- | ----- | ------ | ---- | ------ | ------- |
| T-0001 | Repository Skeleton | SP-001 | EP-001 | ✅ Verified | [`Task-verified-0001.md`](Verified/Task-verified-0001.md) |
| T-0002 | Core Value Types | SP-001 | EP-001 | ✅ Verified | [`Task-verified-0002.md`](Verified/Task-verified-0002.md) |
| T-0003 | Utility Foundation | SP-001 | EP-001 | ✅ Verified | [`Task-verified-0003.md`](Verified/Task-verified-0003.md) |
| T-0004 | Mock Services | SP-001 | EP-001 | ✅ Verified | [`Task-verified-0004.md`](Verified/Task-verified-0004.md) |
| T-0005 | Schema Read/Write | SP-001 | EP-001 | ✅ Verified | [`Task-verified-0005.md`](Verified/Task-verified-0005.md) |
| T-0006 | Project Creation | SP-002 | EP-002 | ✅ Verified | [`Task-verified-0006.md`](Verified/Task-verified-0006.md) |
| T-0007 | Open/Resume | SP-002 | EP-002 | ✅ Verified | [`Task-verified-0007.md`](Verified/Task-verified-0007.md) |
| T-0008 | Save Scene | SP-002 | EP-002 | ✅ Verified | [`Task-verified-0008.md`](Verified/Task-verified-0008.md) |
| T-0009 | External Change Scan | SP-002 | EP-002 | ✅ Verified | [`Task-verified-0009.md`](Verified/Task-verified-0009.md) |
| T-0010 | Git Snapshots | SP-002 | EP-002 | ✅ Verified | [`Task-verified-0010.md`](Verified/Task-verified-0010.md) |
| T-0011 | Swift Interop Prototype | SP-003 | EP-002 | ✅ Verified | [`Task-verified-0011.md`](Verified/Task-verified-0011.md) |
| T-0012 | Identity Service and UUID Provider | SP-003 | EP-003 | ✅ Verified | [`Task-verified-0012.md`](Verified/Task-verified-0012.md) |
| T-0013 | appSupportRoot Directory Bootstrap | SP-003 | EP-003 | ✅ Verified | [`Task-verified-0013.md`](Verified/Task-verified-0013.md) |
| T-0014 | Introduce `src/domain/` and Relocate Slug and TextStats | SP-004 | EP-007 | ✅ Verified | [`Task-verified-0014.md`](Verified/Task-verified-0014.md) |
| T-0015 | Move `AppSupportLayout` from `src/util/` to `src/platform/` | SP-004 | EP-007 | ✅ Verified | [`Task-verified-0015.md`](Verified/Task-verified-0015.md) |
| T-0016 | Merge `MockServicesTests.cpp` into `ResultTests.cpp` | SP-004 | EP-007 | ✅ Verified | [`Task-verified-0016.md`](Verified/Task-verified-0016.md) |
| T-0017 | Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp` | SP-004 | EP-007 | ✅ Verified | [`Task-verified-0017.md`](Verified/Task-verified-0017.md) |
| T-0018 | First Green Build and Full Test Suite Verification | SP-004 | EP-007 | ✅ Verified | [`Task-verified-0018.md`](Verified/Task-verified-0018.md) |
| T-0019 | Add `SnapshotMetadataJson` Schema | SP-005 | EP-007 | ✅ Verified | [`Task-verified-0019.md`](Verified/Task-verified-0019.md) |
| T-0020 | Add `SnapshotMetadataJson` Test Coverage | SP-005 | EP-007 | ✅ Verified | [`Task-verified-0020.md`](Verified/Task-verified-0020.md) |
| T-0021 | Milestone 10 Verification — External Change Scan | SP-005 | EP-007 | ✅ Verified | [`Task-verified-0021.md`](Verified/Task-verified-0021.md) |
| T-0022 | Milestone 11 Verification — Git Snapshots | SP-005 | EP-007 | ✅ Verified | [`Task-verified-0022.md`](Verified/Task-verified-0022.md) |
| T-0023 | Extract `ScriviError.swift` as a Separate File | SP-006 | EP-007 | ✅ Verified | [`Task-verified-0023.md`](Verified/Task-verified-0023.md) |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | SP-006 | EP-007 | ✅ Verified | [`Task-verified-0024.md`](Verified/Task-verified-0024.md) |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | SP-006 | EP-007 | ✅ Verified | [`Task-verified-0025.md`](Verified/Task-verified-0025.md) |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | SP-006 | EP-007 | ✅ Verified | [`Task-verified-0026.md`](Verified/Task-verified-0026.md) |
| T-0027 | Full MVP Loop Integration Test | SP-007 | EP-002 | ✅ Verified | [`Task-verified-0027.md`](Verified/Task-verified-0027.md) |
| T-0028 | `RepairIssueJson` Schema Module | SP-008 | EP-004 | ✅ Verified | [`Task-verified-0028.md`](Verified/Task-verified-0028.md) |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | SP-008 | EP-004 | ✅ Verified | [`Task-verified-0029.md`](Verified/Task-verified-0029.md) |
| T-0030 | Repair Handlers — Manuscript File Operations | SP-009 | EP-004 | ✅ Verified | [`Task-verified-0030.md`](Verified/Task-verified-0030.md) |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | SP-009 | EP-004 | ✅ Verified | [`Task-verified-0031.md`](Verified/Task-verified-0031.md) |
| T-0032 | Integration Tests for `applyRepair` | SP-009 | EP-004 | ✅ Verified | [`Task-verified-0032.md`](Verified/Task-verified-0032.md) |
| T-0033 | Adapter — `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results | SP-009 | EP-004 | ✅ Verified | [`Task-verified-0033.md`](Verified/Task-verified-0033.md) |
| T-0034 | Character Object Schema (`ObjectJson`) | SP-010 | EP-005 | ✅ Verified | [`Task-verified-0034.md`](Verified/Task-verified-0034.md) |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | SP-010 | EP-005 | ✅ Verified | [`Task-verified-0035.md`](Verified/Task-verified-0035.md) |
| T-0036 | Integration Tests for Character Object CRUD | SP-010 | EP-005 | ✅ Verified | [`Task-verified-0036.md`](Verified/Task-verified-0036.md) |
| T-0037 | Remaining Object Types — Location, Item, Rule, Timeline Schemas | SP-011 | EP-005 | ✅ Verified | [`Task-verified-0037.md`](Verified/Task-verified-0037.md) |
| T-0038 | Generalize Object CRUD Facade to All Object Types | SP-011 | EP-005 | ✅ Verified | [`Task-verified-0038.md`](Verified/Task-verified-0038.md) |
| T-0039 | Integration Tests for All Object Types CRUD | SP-011 | EP-005 | ✅ Verified | [`Task-verified-0039.md`](Verified/Task-verified-0039.md) |
| T-0040 | Asset Metadata Sidecar Schema (`AssetMetaJson`) | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0040.md`](Verified/Task-verified-0040.md) |
| T-0041 | `importAsset` / `listAssets` / `removeAsset` Facade Methods | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0041.md`](Verified/Task-verified-0041.md) |
| T-0042 | Integration Tests for Asset Operations | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0042.md`](Verified/Task-verified-0042.md) |
| T-0043 | Comments Schema (`CommentJson`) and Comment Types | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0043.md`](Verified/Task-verified-0043.md) |
| T-0044 | `addComment` / `listComments` / `resolveComment` Facade Methods | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0044.md`](Verified/Task-verified-0044.md) |
| T-0045 | Integration Tests for Comment Operations | SP-012 | EP-005 | ✅ Verified | [`Task-verified-0045.md`](Verified/Task-verified-0045.md) |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | SP-013 | EP-005 | ✅ Verified | [`Task-verified-0046.md`](Verified/Task-verified-0046.md) |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | SP-013 | EP-005 | ✅ Verified | [`Task-verified-0047.md`](Verified/Task-verified-0047.md) |
| T-0048 | `JsonDoc` Double Support — `setDouble` / `getDouble` | SP-014 | EP-006 | ✅ Verified | [`Task-verified-0048.md`](Verified/Task-verified-0048.md) |
| T-0049 | `KeychainSecureStore` — macOS Keychain Implementation | SP-014 | EP-006 | ✅ Verified | [`Task-verified-0049.md`](Verified/Task-verified-0049.md) |
| T-0050 | macOS App Target — Xcode Project Setup | SP-015 | EP-006 | ✅ Verified | [`Task-verified-0050.md`](Verified/Task-verified-0050.md) |
| T-0051 | ScriviEngine Bootstrap — `AppEnvironment` Observable | SP-015 | EP-006 | ✅ Verified | [`Task-verified-0051.md`](Verified/Task-verified-0051.md) |
| T-0052 | SwiftUI Shell — Landing View, Editor View, and Project Flows | SP-015 | EP-006 | ✅ Verified | [`Task-verified-0052.md`](Verified/Task-verified-0052.md) |
| T-0053 | `saveScene` Wiring — ⌘S Save in SwiftUI Editor | SP-016 | EP-006 | ✅ Verified | [`Task-verified-0053.md`](Verified/Task-verified-0053.md) |
| T-0054 | EP-006 Verification — `swift test` + `ctest` Green | SP-016 | EP-006 | ✅ Verified | [`Task-verified-0054.md`](Verified/Task-verified-0054.md) |
| T-0055 | Ubuntu CMake Build — GCC/Clang Green + Gap Document | SP-017 | EP-008 | ✅ Verified | [`Task-verified-0055.md`](Verified/Task-verified-0055.md) |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | SP-018 | EP-008 | ✅ Verified | [`Task-verified-0056.md`](Verified/Task-verified-0056.md) |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | SP-018 | EP-008 | ✅ Verified | [`Task-verified-0057.md`](Verified/Task-verified-0057.md) |
| T-0058 | SecureStore Trade Study — Linux and Windows | SP-018 | EP-008 | ✅ Verified | [`Task-verified-0058.md`](Verified/Task-verified-0058.md) |
| T-0059 | `OpenProjectResult` — Add Scene List | SP-019 | EP-008 | ✅ Verified | [`Task-verified-0059.md`](Verified/Task-verified-0059.md) |
| T-0060 | `openScene` Facade Method — Switch Active Scene | SP-019 | EP-008 | ✅ Verified | [`Task-verified-0060.md`](Verified/Task-verified-0060.md) |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | SP-019 | EP-008 | ✅ Verified | [`Task-verified-0061.md`](Verified/Task-verified-0061.md) |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | SP-019 | EP-008 | ✅ Verified | [`Task-verified-0062.md`](Verified/Task-verified-0062.md) |
| T-0063 | clang-tidy — `readability-braces-around-statements` (142 warnings) | SP-020 | — | ✅ Verified | [`Task-verified-0063.md`](Verified/Task-verified-0063.md) |
| T-0064 | clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings) | SP-020 | — | ✅ Verified | [`Task-verified-0064.md`](Verified/Task-verified-0064.md) |
| T-0065 | clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings) | SP-020 | — | ✅ Verified | [`Task-verified-0065.md`](Verified/Task-verified-0065.md) |
| T-0066 | clang-tidy — Remaining checks (20 warnings) | SP-020 | — | ✅ Verified | [`Task-verified-0066.md`](Verified/Task-verified-0066.md) |
| T-0067 | `CreateSceneRequest` / `CreateSceneResult` types + `ScriviCore::createScene()` facade | SP-021 | EP-009 | ✅ Verified | [`Task-verified-0067.md`](Verified/Task-verified-0067.md) |
| T-0068 | `SceneCreator` implementation — slug generation, file writes, chapter index update | SP-021 | EP-009 | ✅ Verified | [`Task-verified-0068.md`](Verified/Task-verified-0068.md) |
| T-0069 | `CreateChapterRequest` / `CreateChapterResult` types + `ScriviCore::createChapter()` facade | SP-021 | EP-009 | ✅ Verified | [`Task-verified-0069.md`](Verified/Task-verified-0069.md) |
| T-0070 | `ChapterCreator` implementation — slug generation, file writes, manuscript index update, auto first scene | SP-021 | EP-009 | ✅ Verified | [`Task-verified-0070.md`](Verified/Task-verified-0070.md) |
| T-0071 | Integration tests — `createScene` ordering, insert-after-current, `createChapter` with auto-scene | SP-021 | EP-009 | ✅ Verified | [`Task-verified-0071.md`](Verified/Task-verified-0071.md) |
| T-0072 | ScriviCoreAdapter — createScene binding (C++ shim + header) | SP-022 | EP-009 | ✅ Verified | [`Task-verified-0072.md`](Verified/Task-verified-0072.md) |
| T-0073 | ScriviCoreAdapter — createChapter binding (C++ shim + header) | SP-022 | EP-009 | ✅ Verified | [`Task-verified-0073.md`](Verified/Task-verified-0073.md) |
| T-0074 | ScriviEngine.swift — createScene wrapper + CreateSceneResult Swift type | SP-022 | EP-009 | ✅ Verified | [`Task-verified-0074.md`](Verified/Task-verified-0074.md) |
| T-0075 | ScriviEngine.swift — createChapter wrapper + CreateChapterResult Swift type | SP-022 | EP-009 | ✅ Verified | [`Task-verified-0075.md`](Verified/Task-verified-0075.md) |
| T-0076 | Swift interop tests — round-trip createScene and createChapter | SP-022 | EP-009 | ✅ Verified | [`Task-verified-0076.md`](Verified/Task-verified-0076.md) |
| T-0077 | ViewportSceneLoader — load current scene + neighbors until viewport fills + 1 buffer scene each direction | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0077.md`](Verified/Task-verified-0077.md) |
| T-0078 | ManuscriptTextView — NSViewRepresentable wrapping NSTextView; displays concatenated scene segments separated by divider views | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0078.md`](Verified/Task-verified-0078.md) |
| T-0079 | Divider rendering — thin horizontal rule (1pt, system separator color) with no text, no labels | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0079.md`](Verified/Task-verified-0079.md) |
| T-0080 | Auto-save — 1-second debounce Task after last keystroke; scene-exit immediate save; app-resign immediate save + backup placeholder hook | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0080.md`](Verified/Task-verified-0080.md) |
| T-0081 | ⌘↩ key binding — save current scene, call createScene, insert divider, move cursor to new scene segment | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0081.md`](Verified/Task-verified-0081.md) |
| T-0082 | ⌘⇧↩ key binding — save current scene, call createChapter, insert divider, move cursor to new chapter's first scene | SP-023 | EP-009 | ✅ Verified | [`Task-verified-0082.md`](Verified/Task-verified-0082.md) |
| T-0083 | `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order | SP-024 | — | ✅ Verified | [`Task-verified-0083.md`](Verified/Task-verified-0083.md) |
| T-0084 | Navigator title derivation — first line of author text or "Scene X"; chapter section headers | SP-024 | — | ✅ Verified | [`Task-verified-0084.md`](Verified/Task-verified-0084.md) |
| T-0085 | Live title updates — ~300ms debounce on keystroke for currently active scene segment | SP-024 | — | ✅ Verified | [`Task-verified-0085.md`](Verified/Task-verified-0085.md) |
| T-0086 | Click-to-navigate — load selected scene, scroll editor to segment start | SP-024 | — | ✅ Verified | [`Task-verified-0086.md`](Verified/Task-verified-0086.md) |
| T-0087 | Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package | SP-024 | — | ✅ Verified | [`Task-verified-0087.md`](Verified/Task-verified-0087.md) |
| T-0088 | Scroll-down past divider — promote next scene, save departing scene | SP-025 | — | ✅ Verified | [`Task-verified-0088.md`](Verified/Task-verified-0088.md) |
| T-0089 | Scroll-up past divider — promote previous scene, save departing scene | SP-025 | — | ✅ Verified | [`Task-verified-0089.md`](Verified/Task-verified-0089.md) |
| T-0090 | Remove `ScriviCoreAdapter` target from Xcode — update `project.pbxproj` | SP-026 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-026.md` |
| T-0091 | Replace adapter module map with `ScriviCore` plain-C module map (`scrivi.h`) | SP-026 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-026.md` |
| T-0092 | Rewrite `ScriviEngine.swift` — call `scrivi_*` C functions; `scrivi_free()` after each call | SP-026 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-026.md` |
| T-0093 | Update `ScriviInteropTests.swift` — fix `@testable import` to correct module name | SP-026 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-026.md` |
| T-0094 | Verify: Xcode build clean, `ScriviInteropTests` green, `ctest` count unchanged | SP-026 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-026.md` |
| T-0095 | EP-009 verification — all 18 ACs green; 171 ctests green; macOS smoke test | SP-025 | — | ✅ Verified | [`Task-verified-0095.md`](Verified/Task-verified-0095.md) |
| T-0096 | Scroll bar fidelity over the full manuscript extent | SP-034 | EP-011 | 🔵 Backlog | — |
| T-0097 | Implement `deleteScene` C++ facade method — remove scene file + sidecar, update chapter index | SP-027 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-027.md` |
| T-0098 | Implement `deleteChapter` C++ facade method — remove chapter directory, update manuscript index | SP-027 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-027.md` |
| T-0099 | Implement `renameScene(metadataPath, newTitle)` C++ facade method — write `title` to scene sidecar JSON | SP-028 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-028.md` |
| T-0100 | Implement `renameChapter(metadataPath, newTitle)` C++ facade method — write `title` to chapter sidecar JSON | SP-028 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-028.md` |
| T-0101 | Implement `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)` C++ facade method | SP-029 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-029.md` |
| T-0102 | Implement `reorderChapter(chapterID, afterChapterID?)` C++ facade method | SP-029 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-029.md` |
| T-0103 | Wire `deleteScene`, `deleteChapter`, `renameScene`, `renameChapter` through `ScriviEngine.swift` | SP-030 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-030.md` |
| T-0104 | SwiftUI context menu on scene and chapter Navigator rows — right-click (macOS) / long-press (iOS) with Rename and Delete items | SP-030 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-030.md` |
| T-0105 | Rename sheet (focused edit field, pre-populated title, save/cancel) and delete confirmation dialog (warns chapter deletes all scenes) | SP-030 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-030.md` |
| T-0106 | Wire `reorderScene` and `reorderChapter` through `scrivi.h` C API and `ScriviEngine.swift` | SP-031 | EP-010 | ✅ Verified | [`Task-verified-0106.md`](Verified/Task-verified-0106.md) |
| T-0107 | SwiftUI drag-and-drop for scene rows — reorder within chapter and move across chapter boundary | SP-031 | EP-010 | ✅ Verified | [`Task-verified-0107.md`](Verified/Task-verified-0107.md) |
| T-0108 | SwiftUI drag-and-drop for chapter rows — reorder chapter as container carrying all scenes; insertion-line highlight | SP-031 | EP-010 | ✅ Verified | [`Task-verified-0108.md`](Verified/Task-verified-0108.md) |
| T-0109 | Global chapter title toggle + headings in writing surface | SP-032 | EP-010 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-032.md` |
| T-0110 | Navigator title fallback chain + delete-of-open-scene edge case + cursor placement | SP-032 | EP-010 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-032.md` |
| T-0111 | EP-010 acceptance criteria verification + macOS smoke test | SP-032 | EP-010 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-032.md` |
| T-0112 | Cursor position tracking in the manuscript | — | EP-011 | 🔵 Backlog | — |
| T-0113 | EP-011 Behavior Spec — scroll, cursor, separator, focus, delete rules | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0113.md`](Verified/Task-verified-0113.md) |
| T-0114 | All-scenes-in-memory viewport strategy — replace load/release cycle | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0114.md`](Verified/Task-verified-0114.md) |
| T-0115 | Global cursor position tracking + scene start position map | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0115.md`](Verified/Task-verified-0115.md) |
| T-0116 | Correct cursor placement after navigate and delete | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0116.md`](Verified/Task-verified-0116.md) |
| T-0117 | Reliable first-responder transfer — replace `focusManuscriptView` workaround | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0117.md`](Verified/Task-verified-0117.md) |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | SP-033 | EP-011 | 🔵 Backlog | — |
| T-0119 | EP-011 verification — macOS smoke test + deferred EP-010 AC | SP-033 | EP-011 | ✅ Verified | [`Task-verified-0119.md`](Verified/Task-verified-0119.md) |
| T-0120 | Remove toolbar strip and add Project menu | SP-035 | EP-012 | ✅ Verified | [`Task-verified-0120.md`](Verified/Task-verified-0120.md) |
| T-0121 | Add Edit menu with standard responder-chain commands | SP-035 | EP-012 | ✅ Verified | [`Task-verified-0121.md`](Verified/Task-verified-0121.md) |
| T-0122 | Add About menu with About panel and User Manual placeholder | SP-035 | EP-012 | ✅ Verified | [`Task-verified-0122.md`](Verified/Task-verified-0122.md) |
| T-0123 | Verified | SP-035 | EP-012 | ✅ Verified | [`Task-verified-0123.md`](Verified/Task-verified-0123.md) |
| T-0124 | EP-012 Verification | SP-035 | EP-012 | ✅ Verified | [`Task-verified-0124.md`](Verified/Task-verified-0124.md) |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | SP-036 | EP-013 | ⚪ Closed | [`Task-closed-0125.md`](Closed/Task-closed-0125.md) |
| T-0126 | iPadOS two-finger swipe gesture | — | EP-013 | 🔵 Backlog | — |
| T-0127 | EP-013 verification | SP-036 | EP-013 | ⚪ Closed | [`Task-closed-0127.md`](Closed/Task-closed-0127.md) |
| T-0128 | SceneInspectorView Skeleton — Panel, Tab Bar Structure | SP-037 | EP-014 | ✅ Verified | [`Task-verified-0128.md`](Verified/Task-verified-0128.md) |
| T-0129 | Scene Entities Stub Tab — Empty State, Placeholder Add Entity Button | SP-037 | EP-014 | ✅ Verified | [`Task-verified-0129.md`](Verified/Task-verified-0129.md) |
| T-0130 | Inspector Panel Integration into ManuscriptEditorView — Width, Collapse, Expand | SP-037 | EP-014 | ✅ Verified | [`Task-verified-0130.md`](Verified/Task-verified-0130.md) |
| T-0131 | iPhone Exclusion — Inspector Absent on Phone Idiom | SP-037 | EP-014 | ✅ Verified | [`Task-verified-0131.md`](Verified/Task-verified-0131.md) |
| T-0132 | EP-014 Verification | SP-037 | EP-014 | ✅ Verified | [`Task-verified-0132.md`](Verified/Task-verified-0132.md) |
| T-0133 | TimelineStripView Skeleton — Panel Chrome, Horizontal Scroll Container | SP-038 | EP-015 | ✅ Verified | [`Task-verified-0133.md`](Verified/Task-verified-0133.md) |
| T-0134 | Placeholder Graphical Event Markers — Static Stub Layout | SP-038 | EP-015 | ✅ Verified | [`Task-verified-0134.md`](Verified/Task-verified-0134.md) |
| T-0135 | Timeline Integration into ManuscriptEditorView — Docking, Height, Collapse | SP-038 | EP-015 | ✅ Verified | [`Task-verified-0135.md`](Verified/Task-verified-0135.md) |
| T-0136 | iPhone Exclusion — Timeline Absent on Phone Idiom | SP-038 | EP-015 | ✅ Verified | [`Task-verified-0136.md`](Verified/Task-verified-0136.md) |
| T-0137 | EP-015 Verification | SP-038 | EP-015 | ✅ Verified | [`Task-verified-0137.md`](Verified/Task-verified-0137.md) |
| T-0139 | `TimelineMetaJson` schema — read/write `timeline.meta.json` | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0140 | `SceneMetaJson` extension — `storyTime` block read/write | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0141 | `StoryStructureJson` schema — read/write `story-structure.json` | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0142 | `HistoricalEventJson` schema — read/write `historical-events/<id>.json` | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0143 | `ExternalTimelineJson` schema — read/write imported timeline files | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0144 | ScriviCore facade — timeline, story-time, and band-assignment operations | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0145 | ScriviCore facade — story structure, historical events, imported timelines, export | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0146 | C API + `ScriviEngine.swift` — expose all timeline operations | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0147 | Integration tests — timeline persistence round-trip | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0148 | `createProject` update — write `timeline.meta.json` on new project | SP-039 | EP-016 | 🔵 Backlog | — |
| T-0149 | TimelineStripView Rebuild — Horizontal Line, Scene Dots, Manuscript-Order Layout | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0149.md`](Verified/Task-verified-0149.md) |
| T-0150 | Scene Dot Drag — Horizontal Gesture, Story-Time Update | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0150.md`](Verified/Task-verified-0150.md) |
| T-0151 | Time Delta Picker — Amount/Unit/Direction Spinner, Named Anchors, Dismiss Behavior | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0151.md`](Verified/Task-verified-0151.md) |
| T-0152 | Time Delta Picker — Spinner Pre-Population; Anchor Deltas Relative to Previous Scene End | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0152.md`](Verified/Task-verified-0152.md) |
| T-0153 | Scene Dot Tooltip / Popover — Title, Chapter, Human-Readable Story-Time | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0153.md`](Verified/Task-verified-0153.md) |
| T-0154 | Timeline Panel Resize — Drag Top Edge, Dynamic Minimum Height | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0154.md`](Verified/Task-verified-0154.md) |
| T-0155 | Expand Timeline Forward/Backward Popover | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0155.md`](Verified/Task-verified-0155.md) |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | SP-041 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-041.md` |
| T-0157 | Band border drag — proportional resize, persistence | SP-041 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-041.md` |
| T-0158 | Band assignment by drag-up to label — ring color on dot | SP-041 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-041.md` |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | SP-041 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-041.md` |
| T-0160 | Scene dot context menu — full menu | SP-041 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-041.md` |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | SP-042 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-042.md` |
| T-0162 | Imported timeline row — render events as grey row below project row | SP-042 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-042.md` |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | SP-042 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-042.md` |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | SP-042 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-042.md` |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | SP-042 | EP-016 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-042.md` |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | SP-042 | EP-016 | ✅ Verified | [`Task-verified-0166.md`](Verified/Task-verified-0166.md) |
| T-0167 | EP-016 Verification | SP-043 | EP-016 | ✅ Verified | [`Task-verified-0167.md`](Verified/Task-verified-0167.md) |
| T-0168 | Scene Duration, Chain Propagation, Project Timeline Defaults, "Immediately After" | SP-040 | EP-016 | ✅ Verified | [`Task-verified-0168.md`](Verified/Task-verified-0168.md) |
| T-0169 | Hover tooltips for historical event and imported event dots | SP-042 | EP-016 | ✅ Verified | [`Task-verified-0169.md`](Verified/Task-verified-0169.md) |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | SP-042 | EP-016 | ✅ Verified | [`Task-verified-0170.md`](Verified/Task-verified-0170.md) |
| T-0171 | Cluster count badge when ring height exceeds panel height | SP-043 | EP-016 | ✅ Verified | [`Task-verified-0171.md`](Verified/Task-verified-0171.md) |
| T-0172 | Zoom in resolves clusters into individual dots | SP-043 | EP-016 | ✅ Verified | [`Task-verified-0172.md`](Verified/Task-verified-0172.md) |
| T-0173 | Bidirectional Timeline ↔ Scene Navigator selection and highlight | SP-043 | EP-016 | ✅ Verified | [`Task-verified-0173.md`](Verified/Task-verified-0173.md) |
| T-0174 | Cluster aggregate-dot representation redesign (readability) | SP-043 | EP-016 | ✅ Verified | [`Task-verified-0174.md`](Verified/Task-verified-0174.md) |
| T-0175 | Spotlight integration (umbrella) — **superseded by EP-017** | — | EP-017 | ⚪ Superseded | — |
| T-0176 | Resolve design gates — boundary (Option A) & adapter-location reconciliation | SP-044 | EP-017 | ✅ Verified | [`Task-verified-0176.md`](Verified/Task-verified-0176.md) |
| T-0177 | Define indexable record schema (fields/attributes per item type) | SP-044 | EP-017 | ✅ Verified | [`Task-verified-0177.md`](Verified/Task-verified-0177.md) |
| T-0178 | ScriviCore `extractSearchableText(projectPath) -> JSON` read-only facade | SP-044 | EP-017 | ✅ Verified | [`Task-verified-0178.md`](Verified/Task-verified-0178.md) |
| T-0179 | Unit + integration tests for the indexing facade against a fixture project | SP-044 | EP-017 | ✅ Verified | [`Task-verified-0179.md`](Verified/Task-verified-0179.md) |
| T-0180 | Spotlight/Core Spotlight entitlements + project config | SP-045 | EP-017 | ✅ Verified | [`Task-verified-0180.md`](Verified/Task-verified-0180.md) |
| T-0181 | `ScriviEngine` Swift API to fetch indexable records (calls the facade) | SP-045 | EP-017 | ✅ Verified | [`Task-verified-0181.md`](Verified/Task-verified-0181.md) |
| T-0182 | Donate `CSSearchableItem`s on project open/save; delete-by-domain on close | SP-045 | EP-017 | ✅ Verified | [`Task-verified-0182.md`](Verified/Task-verified-0182.md) |
| T-0183 | Markdown→plain-text extraction for body indexing | SP-045 | EP-017 | ✅ Verified | [`Task-verified-0183.md`](Verified/Task-verified-0183.md) |
| T-0184 | Deep-Link — Spotlight Result Continuation (per-window model) | SP-045 | EP-017 | ✅ Verified | [`Task-verified-0184.md`](Verified/Task-verified-0184.md) |
| T-0185 | New Spotlight importer app-extension target (Xcode-authored) | SP-046 | EP-017 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-046.md` |
| T-0186 | Link ScriviCore into the extension (Option A build graph) | SP-046 | EP-017 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-046.md` |
| T-0187 | Importer emits Spotlight attributes from facade JSON | SP-046 | EP-017 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-046.md` |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | SP-046 | EP-017 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-046.md` |
| T-0189 | End-to-end Spotlight verification (donor search, deep-link tap, donations succeed) | SP-047 | EP-017 | ✅ Verified | [`Task-verified-0189.md`](Verified/Task-verified-0189.md) |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | SP-047 | EP-017 | ✅ Verified | [`Task-verified-0190.md`](Verified/Task-verified-0190.md) |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3) | SP-048 | EP-018 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-048.md` |
| T-0192 | Extract `ProjectSession`; move per-project state off `AppEnvironment` | SP-048 | EP-018 | ✅ Verified | [`Task-verified-0192.md`](Verified/Task-verified-0192.md) |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` | SP-048 | EP-018 | ✅ Verified | [`Task-verified-0193.md`](Verified/Task-verified-0193.md) |
| T-0194 | Per-window project model — AppKit NSWindow per project | SP-049 | EP-018 | ✅ Verified | [`Task-verified-0194.md`](Verified/Task-verified-0194.md) |
| T-0195 | Session manifest persistence + restore all previously-open windows | SP-049 | EP-018 | ✅ Verified | [`Task-verified-0195.md`](Verified/Task-verified-0195.md) |
| T-0196 | Deep-link on per-window model + scene-`ID` fix + EP-018 verification | SP-050 | EP-018 | ✅ Verified | [`Task-verified-0196.md`](Verified/Task-verified-0196.md) |
| T-0197 | Enable Core Spotlight donation on iOS/iPadOS (+ deep-link/bookmark consumer) | — | EP-017 | 🔵 Backlog | — |
| T-0198 | Undo/redo + copy buffers design doc & trade studies | SP-051 | EP-019 | ✅ Verified | [`Task-verified-0198.md`](Verified/Task-verified-0198.md) |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | SP-051 | EP-019 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-051.md` |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | SP-051 | EP-019 | ✅ Verified | [`Task-verified-0200.md`](Verified/Task-verified-0200.md) |
| T-0201 | `HistoryService` core (in-memory, linear) | SP-052 | EP-019 | ✅ Verified | [`Task-verified-0201-0203.md`](Verified/Task-verified-0201-0203.md) |
| T-0202 | `scrivi_history_*` C ABI | SP-052 | EP-019 | ✅ Verified | [`Task-verified-0201-0203.md`](Verified/Task-verified-0201-0203.md) |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | SP-052 | EP-019 | ✅ Verified | [`Task-verified-0201-0203.md`](Verified/Task-verified-0201-0203.md) |
| T-0204 | `HistoryCapture` + commit triggers | SP-053 | EP-019 | ✅ Verified | [`Task-verified-0204-0206.md`](Verified/Task-verified-0204-0206.md) |
| T-0205 | Apply path + `allowsUndo=false` + ⌘Z routing | SP-053 | EP-019 | ✅ Verified | [`Task-verified-0204-0206.md`](Verified/Task-verified-0204-0206.md) |
| T-0206 | Barriers on structural operations | SP-053 | EP-019 | ✅ Verified | [`Task-verified-0204-0206.md`](Verified/Task-verified-0204-0206.md) |
| T-0207 | / T-0208 / T-0209 — ✅ Verified (2026-07-09) | SP-054 | EP-019 | ✅ Verified | [`Task-verified-0207-0209.md`](Verified/Task-verified-0207-0209.md) |
| T-0208 | Capacity + settings + UI | SP-054 | EP-019 | ✅ Verified | [`Task-verified-0207-0209.md`](Verified/Task-verified-0207-0209.md) |
| T-0209 | Session-boundary warning popup | SP-054 | EP-019 | ✅ Verified | [`Task-verified-0207-0209.md`](Verified/Task-verified-0207-0209.md) |
| T-0210 | / T-0211 / T-0212 — ✅ Verified (2026-07-13) | SP-055 | EP-019 | ✅ Verified | [`Task-verified-0210-0212.md`](Verified/Task-verified-0210-0212.md) |
| T-0211 | Inline fork popover (Trade T2 core interaction) | SP-055 | EP-019 | ✅ Verified | [`Task-verified-0210-0212.md`](Verified/Task-verified-0210-0212.md) |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | EP-019 | ✅ Verified | [`Task-verified-0210-0212.md`](Verified/Task-verified-0210-0212.md) |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | SP-056 | EP-019 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-056.md` |
| T-0214 | Buffer UX (HUD/palette/menus) + paste/cut history integration | SP-056 | EP-019 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-056.md` |
| T-0215 | History panel (management surface) | SP-057 | EP-019 | ⚪ Superseded | — |
| T-0216 | Perf/integration fixtures + gitignore migration | SP-057 | EP-019 | ⛔️ Removed as OBE | — |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | **SP-094** (⬅ SP-057 superseded) | EP-019 | ✅ Verified 2026-08-11 | ⚠️ *unfiled* — see [`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md) |
| T-0218 | Qt6/QML CMake skeleton | SP-058 | EP-020 | ✅ Verified | [`Task-verified-0218-0222.md`](Verified/Task-verified-0218-0222.md) |
| T-0219 | `ScriviBridge` | SP-058 | EP-020 | ✅ Verified | [`Task-verified-0218-0222.md`](Verified/Task-verified-0218-0222.md) |
| T-0220 | Hello-ScriviCore slice | SP-058 | EP-020 | ✅ Verified | [`Task-verified-0218-0222.md`](Verified/Task-verified-0218-0222.md) |
| T-0221 | Docker + Xvfb + VNC | SP-058 | EP-020 | ✅ Verified | [`Task-verified-0218-0222.md`](Verified/Task-verified-0218-0222.md) |
| T-0222 | CI | SP-058 | EP-020 | ✅ Verified | [`Task-verified-0218-0222.md`](Verified/Task-verified-0218-0222.md) |
| T-0223 | XDG `appSupportRoot` resolver  *(AC4 foundation)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0224 | Recent-projects store  *(AC5)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0225 | `ScriviBridge` lifecycle methods  *(AC2, AC4)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0226 | Landing view QML  *(AC1)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0227 | New Project flow  *(AC2)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0228 | Docker/CI | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0229 | `EncryptedFileSecureStore` (Linux persistent identity)  *(AC4 — added mid-sprint, user-approved)* | SP-059 | EP-021 | ✅ Verified | [`Task-verified-0223-0229.md`](Verified/Task-verified-0223-0229.md) |
| T-0230 | `ScriviBridge.openProject(projectRootPath, appSupportRoot)`  *(AC3)* | SP-060 | EP-021 | ✅ Verified | [`Task-verified-0230-0233.md`](Verified/Task-verified-0230-0233.md) |
| T-0231 | Open flow (QML)  *(AC3)* | SP-060 | EP-021 | ✅ Verified | [`Task-verified-0230-0233.md`](Verified/Task-verified-0230-0233.md) |
| T-0232 | Close → landing  *(AC6)* | SP-060 | EP-021 | ✅ Verified | [`Task-verified-0230-0233.md`](Verified/Task-verified-0230-0233.md) |
| T-0233 | Full-loop verify + CI  *(AC7, AC8)* | SP-060 | EP-021 | ✅ Verified | [`Task-verified-0230-0233.md`](Verified/Task-verified-0230-0233.md) |
| T-0234 | `[Linux]` **Shell flip** — `main.cpp` → `QApplication`+`QMainWindow` host, landing QML re-hosted via `QQuickWidget`; keep context props + `QFileDialog… | SP-061 | EP-021 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-061.md` |
| T-0235 | `[Linux]` **Editor shell + read-only continuous viewport** — one `QPlainTextEdit`/`QTextDocument` (read-only, undo disabled), all scene bodies via `op… | SP-061 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-061.md` |
| T-0236 | `[Linux]` **Scene navigator** — ordered list + chapter grouping + live titles; click selects + scrolls viewport; display/select only (structure editin… | SP-061 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-061.md` |
| T-0237 | `[Linux]` **Verify AC1/AC2 + headless scene-load smoke** — multi-scene fixture asserts all bodies loaded, wired to CI; VNC click-through | SP-061 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-061.md` |
| T-0238 | `[Linux]` **Editable viewport + dirty tracking** — `setReadOnly(false)`, edits mapped to owning scene via offset map, per-scene dirty flags, non-edita… | SP-062 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-062.md` |
| T-0239 | `[Linux]` **Per-scene auto-save** — `scrivi_save_scene` on ~1.5s debounce + scene-switch + close/app-quit (VNC foreground-quit path); real `.md` on di… | SP-062 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-062.md` |
| T-0240 | `[Linux]` **⌘↩ create scene** in-editor (Linux: **Ctrl+Return**) — save current, create-scene, insert boundary, caret into new segment, update offset… | SP-062 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-062.md` |
| T-0241 | `[Linux]` **⌘⇧↩ create chapter** in-editor (Linux: **Ctrl+Shift+Return**) — save current, create-chapter, insert boundary, caret into new chapter's fi… | SP-062 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-062.md` |
| T-0242 | `[Linux]` **Verify AC3 + headless save smoke** — edit→`save_scene`→reopen asserts new bytes; VNC type/switch/⌘↩/⌘⇧↩/quit-save | SP-062 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-062.md` |
| T-0243 | `[Linux]` **Scroll → active-scene promotion** — visible region → active scene via offset map; boundary crossing promotes + saves departing scene | SP-063 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-063.md` |
| T-0244 | `[Linux]` **Navigator ↔ scroll sync** — highlight follows scroll; navigator click takes caret to the clicked scene's start (scrolls into view); no fee… | SP-063 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-063.md` |
| T-0245 | `[Linux]` **Verify AC1/AC4 (scroll)** — VNC scroll-through tracks active scene + highlight; departing scenes saved; navigator click lands caret at the… | SP-063 | EP-022 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-063.md` |
| T-0246 | `[Linux]` **Cursor placement + focus** — editor takes focus on open, sensible caret, correct caret across boundaries (no jump-to-start), enforce non-e… | SP-064 | EP-022 | 🟠 Implemented - Not Verified | — |
| T-0247 | `[Linux]` **Quit→reopen surface restore** — persist active scene + anchor/focus/scroll on close; apply `openProject` `restored{}` on reopen | SP-064 | EP-022 | 🔵 Backlog | — |
| T-0248 | `[Linux]` **EP-022 verification + close prep** — full write→save→switch→scroll→quit→reopen loop VNC-verified; `.md` on disk; AC1–AC7 checklist; restor… | SP-064 | EP-022 | 🔵 Backlog | — |
| T-0249 | `[Linux]` **Manuscript navigation gestures** — Page Forward / Page Backward + jump to absolute manuscript start / end. Gestures/keystrokes **undecided… | SP-063 | EP-022 | 🔵 Backlog | — |
| T-0250 | `delete_scene`/`delete_chapter` `ScriviBridge` wrappers — `ScriviBridge::deleteScene`/`deleteChapter` (`ScriviBridge.cpp:195/211`) following the `crea… | SP-065 | EP-023 | ✅ Verified | [`Task-verified-0250-0253.md`](Verified/Task-verified-0250-0253.md) |
| T-0251 | Navigator context menu + delete confirmation dialogs — `EditorShell::onNavigatorContextMenu` (custom context-menu policy) → **Delete Scene** / **Delet… | SP-065 | EP-023 | ✅ Verified | [`Task-verified-0250-0253.md`](Verified/Task-verified-0250-0253.md) |
| T-0252 | Removal splice + delete-of-active-scene navigation — `SceneDocument::removeScene`/`removeChapter` surgically delete the scene body(ies) + one adjoinin… | SP-065 | EP-023 | ✅ Verified | [`Task-verified-0250-0253.md`](Verified/Task-verified-0250-0253.md) |
| T-0253 | Verify AC1/AC2 + delete headless smoke — new `scene_delete_smoke` (`.cpp`+`.sh`, CMake target `scrivi_linux_scene_delete_smoke`, CI step): 4-scene/2-c… | SP-065 | EP-023 | ✅ Verified | [`Task-verified-0250-0253.md`](Verified/Task-verified-0250-0253.md) |
| T-0254 | Rename bridge wrappers + `chapterMetadataPath` capture — `ScriviBridge::renameScene`/`renameChapter` (`scrivi_rename_scene`/`_chapter`, `parseEnvelope… | SP-066 | EP-023 | ✅ Verified | [`Task-verified-0254-0257.md`](Verified/Task-verified-0254-0257.md) |
| T-0255 | Context-menu Rename + inline edit + live label/heading — `EditorShell::onNavigatorContextMenu` gained **Rename Scene…/Rename Chapter…** → `QInputDialo… | SP-066 | EP-023 | ✅ Verified | [`Task-verified-0254-0257.md`](Verified/Task-verified-0254-0257.md) |
| T-0256 | Close I-0062 — live new-chapter heading — `SceneDocument::chapterHeadingText` derives the ordinal "Chapter N" from segment order (custom title wins),… | SP-066 | EP-023 | ✅ Verified | [`Task-verified-0254-0257.md`](Verified/Task-verified-0254-0257.md) |
| T-0257 | Verify AC3 + rename headless smoke — new `scene_rename_smoke` (`.cpp`+`.sh`, CMake target, CI step): Cases A–C (scene rename no-doc-change, chapter re… | SP-066 | EP-023 | ✅ Verified | [`Task-verified-0254-0257.md`](Verified/Task-verified-0254-0257.md) |
| T-0258 | `reorderChapter`/`reorderScene` ScriviBridge wrappers | SP-067 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-067.md` |
| T-0259 | Scene drag-reorder (EP-023 AC4) | SP-067 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-067.md` |
| T-0260 | Scene drag-reorder persistence | SP-067 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-067.md` |
| T-0261 | I-0064 chapter-split at the caret | SP-067 | EP-023 | ⚪ Superseded | — |
| T-0262 | I-0063 chapter renumber | SP-067 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-067.md` |
| T-0263 | SP-067 verification pass | SP-067 | EP-023 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-067.md` |
| T-0264 | **P1: `FileSystem::renamePath` port method + `LocalFileSystem` impl** — no-clobber + missing-source guards + `std::filesystem::rename` (atomic-within-… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0265 | **P2: `util/OrderKey`** — fractional (LexoRank-style) base-62 order keys: `keyBetween/keyBefore/keyAfter/isOrderKey`. 8 property tests (3119 assertion… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0266 | **P2: `manuscript/ChapterIndex`** — disk-authoritative helpers + `rebuildIndexIfInconsistent` (open-time self-heal). Unit tests `[ChapterIndex]` (5 ca… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0267 | **P2: `ChapterCreator` order-key slug** — collision-free (**fixes I-0072**); `ProjectCreator` initial `chapter-001` kept. Integration regression `[I-0… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0268 | **P2: disk-authoritative order + reorder** — `ManuscriptOrderResolver` folder-key sort (B3); `ChapterReorderer` = `keyBetween` + `renamePath` the one… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0269 | **P2: open-time index self-heal** — `rebuildIndexIfInconsistent` wired into `ProjectOpener`; I-0072-corrupt index rebuilt from disk, idempotent. Integ… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0270 | **P3: legacy-project migration** — `migrateChapterOrderKeys` assigns fresh ascending order-keys in index-array order via `renameChapterFolder`, then s… | SP-069 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-069.md` |
| T-0271 | **`SceneRef` filename-only schema + serde** — `{metadataFilename}` (drops `sceneID`+path); dual-scheme read keeps a legacy `metadataPath`/`sceneID` re… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0272 | **`util/OrderKey` scene filenames in `SceneCreator`/`ChapterCreator`/`ProjectCreator`** — new scenes get `<orderKey>-<slug>` filenames, filename-only… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0273 | **`manuscript/SceneIndex`** — `listScenesByOrder` (folder-scan, order-key sort, id from sidecar), `sceneMetaRelPath`/`chapterDirOf`, `rebuildChapterSc… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0274 | **`SceneReorderer` relocates files** — cross-chapter move = `renamePath` both files into the dest folder + between-neighbours order key + rebuild both… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0275 | **Simplify `renameChapterFolder`** — remove per-scene `metadataPath`/`contentPath` rewriting (filename-only refs need none); only the chapter `slug` s… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0276 | **Scene consumers updated** — `SceneDeleter`, `SceneRenamer.findSceneMetadataPath`, `ManuscriptOrderResolver`, `ProjectValidator`, `ExternalChangeScan… | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0277 | **Scene migration + orphan repair wired into `ProjectOpener` (before validation)** + 5 integration tests `[EP-027][scenes]`. Full suite 298/298 macOS. | SP-070 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-070.md` |
| T-0278 | (never assigned — ID skipped) | — | — | ⚪ Never assigned | — |
| T-0279 | **`CreateChapterRequest.afterChapterID`** (empty = append). | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0280 | **`ChapterCreator` create-in-place** — `(lo,hi)` window + `keyBetween`; guard empty-key error. | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0281 | **C ABI + facade** — `scrivi_create_chapter` trailing `afterChapterID` (additive; `scrivi.h` updated); facade passthrough. | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0282 | **`ScriviBridge::createChapter(..., afterChapterID = {})`.** | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0283 | **`EditorShell` split rewrite** — one create-in-place call; drop reorder + stale re-resolve; C1/C3/C4 fixes; remove `[split]` debug logs. | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0284 | **Tests** — create-in-place unit test (create after Ch2 → `chapter-c`, **never** `chapter-w`; K0 path valid); split repro updated to the one-call form… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0285 | **2nd defect (found in VNC verify): same-named scene stolen on reopen.** After a follower reordered OUT of a chapter, `rebuildChapterScenesIfInconsist… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0286 | **App hardening:** `EditorShell` split now **checks every `saveScene`/`reorderScene` return and aborts+warns on the first failure** (was silently cont… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0287 | **Remove the split confirmation dialog** — Ctrl+Shift+Return IS the approval. A modal "Split into New Chapter?" prompt on every break destroyed drafti… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0288 | **I-0075 — arrows cross scene/chapter boundaries.** `normalizeCaret` snapped to the *nearest* editable edge (tie → previous), so Down at a scene end /… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0289 | **Strip `[split]` diagnostic logging** — removed the verbose segment-dump + per-call `qInfo` (their job is done). Kept the `failSplit` abort + a singl… | SP-071 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-071.md` |
| T-0290 | I-0078 — macOS `createChapter` wrapper gains `afterChapterID` (append default) | SP-072 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-072.md` |
| T-0291 | I-0076 — `parseSceneMeta` bares `content.path`; `migrateScenes` normalises stale sidecars | SP-072 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-072.md` |
| T-0292 | I-0077 — `migrateChapterOrderKeys` path-folder fallback on id mismatch | SP-072 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-072.md` |
| T-0293 | 2 regression tests in `SceneIdentityTests.cpp` (I-0076 full-path normalise; I-0077 mismatch migrates), each verified RED-without-fix | SP-072 | EP-027 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-072.md` |
| T-0294 | [Linux] `NavigatorTree` chapter drag | SP-073 | EP-023 | ✅ Verified | [`Task-verified-0294-0297.md`](Verified/Task-verified-0294-0297.md) |
| T-0295 | [Linux] `EditorShell::onChapterDropped` + `SceneDocument::moveChapter` | SP-073 | EP-023 | ✅ Verified | [`Task-verified-0294-0297.md`](Verified/Task-verified-0294-0297.md) |
| T-0296 | [Linux] Headless chapter-reorder smoke | SP-073 | EP-023 | ✅ Verified | [`Task-verified-0294-0297.md`](Verified/Task-verified-0294-0297.md) |
| T-0297 | [Linux] Full EP-023 verify + Epic close prep | SP-073 | EP-023 | ✅ Verified | [`Task-verified-0294-0297.md`](Verified/Task-verified-0294-0297.md) |
| T-0298 | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | SP-074 | EP-028 | 🟠 Implemented - Not Verified | — |
| T-0299 | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake | SP-074 | EP-028 | 🟠 Implemented - Not Verified | — |
| T-0300 | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (fixes I-0083) | SP-074 | EP-028 | 🟠 Implemented - Not Verified | — |
| T-0301 | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | SP-074 | EP-028 | 🟠 Implemented - Not Verified | — |
| T-0302 | `[Apple]` `ScriviEngine.mergeScene` / `mergeChapter` wrappers — call `scrivi_merge_scene` / `scrivi_merge_chapter`, decode the JSON envelopes into `Me… | SP-075 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-075.md` |
| T-0303 | `[Apple]` Point `ManuscriptTextView.handleMergeScene` / `handleMergeChapter` at the engine wrappers (replace the `ViewportSceneLoader` in-memory reass… | SP-075 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-075.md` |
| T-0304 | `[Linux]` `ManuscriptEditor::keyPressEvent` — detect `Ctrl-Backspace` (scene) / `Ctrl-Shift-Backspace` (chapter) at the start of a scene/chapter and e… | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0305 | `[Linux]` `ScriviBridge::mergeScene` / `mergeChapter` `Q_INVOKABLE`s (peers to `reorderScene`/`reorderChapter`) calling the C ABI + decoding the envel… | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0306 | `[Linux]` `EditorShell` merge slots (`onMergeSceneRequested` / `onMergeChapterRequested`): resolve caret→segment, guard the no-op cases, call the brid… | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0307 | Update `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md` — merge is now supported on both platforms (§8 clarified + new §8.1). | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0308 | `[Linux]` **Faint between-scene separator rule** — `ManuscriptEditor::paintEvent` overlay: 1px theme-aware (`QPalette::Mid`) hairline, inset 20px, cen… | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0309 | `[Linux]` **`Ctrl+Return` mid-scene split (macOS `⌘↩` parity)** — split head/tail at the caret (was: always append empty). Save head→current + tail→ne… | SP-076 | EP-028 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-076.md` |
| T-0310 | `[Linux]` **Menu bar scaffold + File/Edit** — build the `QMenuBar` on `ScriviWindow` (File: New/Open/Close/Quit; Edit: Cut/Copy/Paste). Wire File to l… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0311 | `[Linux]` **Scene/Chapter menus** — add Scene ▸ Split/Merge and Chapter ▸ Split/Merge; add public `EditorShell` trigger methods (`splitScene`/`mergeSc… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0312 | `[Linux]` **Project ▸ Settings… stub dialog** — a `QDialog` with a title + "coming soon" placeholder (no settings backend). Menu item enabled when the… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0313 | `[Linux]` **Verify chapter-merge live over VNC via Chapter ▸ Merge** (closes EP-028 AC5/AC6 Linux) + strip the TEMP key/guard diagnostics in `Manuscri… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0314 | `[Linux]` **File ▸ New Project opens the New Project panel** — the menu action now `showLanding()` + emits `ShellController::newProjectRequested`; a `… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0315 | `[Linux]` **File ▸ Open Project shows the folder picker** — the menu action now emits `ShellController::openProjectRequested`; the `Landing.qml` `Conn… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0316 | `[Linux]` **Flush-safe editor-leaving paths + remove the raw "‹ Close" button.** File ▸ Close/New/Open left the editor via `showLanding()` with **no f… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0317 | `[Linux]` **Ctrl+W closes the project** — bind `QKeySequence::Close` (Ctrl+W on Linux) to the Close Project menu action; standard first-order "close t… | SP-077 | — | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-077.md` |
| T-0318 | `[Linux]` **Inspector panel widget + Scene Entities stub tab** — new `SceneInspector` `QWidget` (`SceneInspector.cpp/.hpp`): a `QTabWidget` with one *… | SP-078 | EP-024 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-078.md` |
| T-0319 | `[Linux]` **Dock the panel in `EditorShell` + hide/show plumbing** — added `SceneInspector` as the **third pane** of the existing `QSplitter(Horizonta… | SP-078 | EP-024 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-078.md` |
| T-0320 | `[Linux]` **View ▸ Show Inspector menu toggle (Ctrl+Alt+I) + verify** — added a **View** menu to the SP-077 `QMenuBar` with a **checkable "Show Inspec… | SP-078 | EP-024 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-078.md` |
| T-0321 | `[Linux]` **`ScriviBridge::getTimeline` (+ `getSceneStoryTime`) invokables** — `getTimeline` → `scrivi_get_timeline` (epoch label + meta); `getSceneSt… | SP-079 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-079.md` |
| T-0322 | `[Linux]` **`TimelinePanel` widget** — `QWidget` (`TimelinePanel.cpp/.hpp`): horizontal strip painting a baseline + **one dot per scene** by story-tim… | SP-079 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-079.md` |
| T-0323 | `[Linux]` **Dock as the editor's bottom strip + View ▸ Show Timeline toggle** — wrapped the 3-pane horizontal `splitter_` and `timeline_` in an outer… | SP-079 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-079.md` |
| T-0324 | `[Linux]` **Dot↔navigator bidirectional selection + tooltip + verify** — dot click → `sceneClicked(sceneID)` → `moveCaretToSegment` + focus; active-sc… | SP-079 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-079.md` |
| T-0325 | `[Linux]` **`ScriviBridge::setSceneStoryTime` invokable** — → `scrivi_set_scene_story_time(root, sceneID, offsetMs, source, gapMs, durationMs, duratio… | SP-080 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-080.md` |
| T-0326 | `[Linux]` **`TimelinePanel` dot drag** — press-on-dot → horizontal drag (4px threshold) live-moves the dot (`dragX_`) → `dotDragged(sceneID, offsetFor… | SP-080 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-080.md` |
| T-0327 | `[Linux]` **`TimeDeltaPicker` dialog** (`TimeDeltaPicker.cpp/.hpp`) — amount spinbox + unit combo (Minutes…Years) + direction (Later/Before) seeded vi… | SP-080 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-080.md` |
| T-0328 | `[Linux]` **Wire drag/menu → picker → commit + chain propagation** — `EditorShell::showTimeDeltaPicker` seeds from the `reloadTimeline` offset/duratio… | SP-080 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-080.md` |
| T-0329 | `[Linux]` **Story-structure bridge invokables** — `getStoryStructure`/`setStoryStructure`/`updateBandLayout`/`removeStoryStructure` + `assignSceneToBa… | SP-081 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-081.md` |
| T-0330 | `[Linux]` **Band overlay painting + Structure selector** — `TimelinePanel::setBands` + `paintEvent` paints translucent colored proportional bands + la… | SP-081 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-081.md` |
| T-0331 | `[Linux]` **Band border drag (re-proportion + persist)** — border zone hit-tested first (`borderIndexNearX`); drags proportion between adjacent bands… | SP-081 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-081.md` |
| T-0332 | `[Linux]` **Scene→band assignment (drag-up + context menu) + verify** — a dot dragged up → `sceneAssignedToBand` (`bandIndexAtX` at release) + a color… | SP-081 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-081.md` |
| T-0333 | `[Linux]` **`TimelinePanel` zoom model** — `zoom_` (≥1) + `panFraction_` threaded through `xForOffset`/`offsetForX` (band geometry follows for free);… | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0334 | `[Linux]` **`+`/`−` zoom control + horizontal scrollbar** — two `QToolButton`s at the strip's **bottom-right** (`+` left, `−` right) → `zoomInStep`/`z… | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0335 | `[Linux]` **Pan by background drag** — a press on the empty area (no dot/border) with `zoom_ > 1` → `DragMode::Pan` (closed-hand cursor), drag adjusts… | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0336 | `[Linux]` Time Delta Picker — anchor to ANY scene (resolved once to a manual offset; no schema change) | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0337 | `[Linux]` Story bands wrap the main storyline `[0, last-end]`, not the whole strip; zoom with the timeline; flashback stays assignable | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0338 | `[Linux]` Persist timeline zoom + pan per project (INI under app-support so it survives the `--rm` container) | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0339 | `[Linux]` Truncate long scene titles in the picker's anchor combo (cap width + elide, full title in tooltip) | SP-083 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-083.md` |
| T-0340 | `[Linux]` **Timeline-events bridge invokables** — `ScriviBridge` Qt wrappers over the complete EP-016 C ABI (historical CRUD + import/update-offset/vi… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0341 | `[Linux]` **Historical-event dots + author/edit/delete** — `#C8A97A` dots on the project row (distinct from scene accent + imported grey, §7.2), dragg… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0342 | `[Linux]` **Imported-timeline rows + epoch-offset dialog + hide/show** — one grey row below the project row per import (source label, per-source `assi… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0343 | `[Linux]` **Export timeline** — Export Timeline… (§7.9) → `exportProjectTimeline` → `QFileDialog` save-as writes `.scrivi-timeline.json` (scene + hist… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0344 | `[Linux]` **Wire-up + `timeline_events_smoke` + verify** — `reloadTimeline`/`reloadImportedTimelines` load historical events + imported rows; new head… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0345 | `[Linux]` **File ▸ Import / Export Timeline… menu items** (user request 2026-07-24) — import/export are file ops, so they join File (below Close Proje… | SP-082 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-082.md` |
| T-0346 | `[Linux]` **Clustering pass + aggregate-dot model** — `computeClusters()` groups project-row members (scene + historical dots) whose current-zoom pixe… | SP-084 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-084.md` |
| T-0347 | `[Linux]` **Aggregate-dot paint** — `paintAggregate` draws a slightly-larger core (FR-031) + centred member **count** + a **segmented arc ring** (`360… | SP-084 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-084.md` |
| T-0348 | `[Linux]` **Hover fan-out + zoom-resolve + member interaction** — `updateHoverFan` fans an aggregate's members into the hexagonal ring (`fanOutMemberP… | SP-084 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-084.md` |
| T-0349 | `[Linux]` **Persistence sweep + full EP-025 verify + `timeline_cluster_smoke` + Epic close** — new headless `timeline_cluster_smoke` (N co-located → o… | SP-084 | EP-025 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-084.md` |
| T-0350 | Design doc: fragment schema, extract/paste-splice/cut-merge, buffer-schema evolution, trades T1–T4, milestone breakdown | SP-085 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-085.md` |
| T-0351 | Extract fragment (`scrivi.fragment.v1`) | SP-086 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-086.md` |
| T-0352 | Paste-splice | SP-087 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-087.md` |
| T-0353 | Cut-merge | SP-088 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-088.md` |
| T-0354 | **Pass A** — system-clipboard cross-boundary Cut/Copy/Paste + heading refusal + **barrier** history (AC1/2/3/5/7) | SP-089 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-089.md` |
| T-0355 | **Pass B** — structured copy buffers (AC4; `buffers.json` gains optional `fragment`, T4=A) | SP-089 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-089.md` |
| T-0356 | **Reversible structured undo (AC6)** — extend C++ `HistoryService` for `structuredCut`/`structuredPaste` inverse-op undo/redo + tests (must also resto… | SP-089 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-089.md` |
| T-0357 | **Title-capture cut/paste + chapter promotion** (user ruling 2026-07-29, supersedes §4.3 for chapter-crossing cuts) — cross-chapter cut **promotes the… | SP-089 | EP-029 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-089.md` |
| T-0358 | [ScriviCore] OrderKey caps-only generation + dotted keys + rebalance | — | EP-027 | ✅ Verified | [`Task-verified-0358.md`](Verified/Task-verified-0358.md) |
| T-0359 | `InspectorCard` protocol + registry + `CardContext` | SP-090 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-090.md` |
| T-0360 | `inspector-layout.json` schema + load/save + unknown-`typeID` skip | SP-090 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-090.md` |
| T-0361 | Bottom tabs + persisted `selectedTab` + menu hide/show; replace `SceneInspectorView` | SP-090 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-090.md` |
| T-0362 | Card stack: add/remove/reorder, collapse, per-stack sort, "apply to all scenes" | SP-090 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-090.md` |
| T-0363 | `tags` + `todo` cards | SP-091 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-091.md` |
| T-0364 | `outline` card | SP-091 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-091.md` |
| T-0365 | `sources` card (aggregate) — `cites` type ✅ SP-096, `source` kind ✅ SP-098/T-0406; **only the card remains** | SP-102 | EP-031 | 🔵 Backlog | — |
| T-0366 | `history` card — windowed tree, branches, stale badges, purge (**supersedes T-0215**) | SP-092 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-092.md` |
| T-0367 | Properties tab — field-driven view | SP-092 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-092.md` |
| T-0368 | Card failure isolation + inline warning | SP-092 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-092.md` |
| T-0369 | EP-030 verification + Epic close prep | SP-094 | EP-030 | 🔵 Backlog | — |
| T-0370 | `ObjectKind` additions (8) + `objectKindSubdir` + schema table; retire `timeline`; world-scoped kinds gated | SP-095 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-095.md` |
| T-0371 | `WorldObjectFields`: `subtitle`, `image`, `worldID` | SP-095 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-095.md` |
| T-0372 | `objects/index.json` + `findByID` over index | SP-095 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-095.md` |
| T-0373 | `relation-types.json` + `canonicalDirection` + `symmetric` | SP-096 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-096.md` |
| T-0374 | `relationships.jsonl` append-log + tombstones + torn-line recovery | SP-096 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-096.md` |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric) | SP-096 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-096.md` |
| T-0376 | Compaction at 30% / 1,000 tombstones | SP-096 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-096.md` |
| T-0377 | Cascade-prune on object **and** scene delete + load-time repair | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects` | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0379 | `scrivi_promote_object` (item↔artifact) | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0380 | ⚠️ Pending-vs-dangling distinction + frozen graph toward unavailable worlds | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0381 | `.scrivworld` package + `world.json` + world index + `scrivi_create_world` | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0382 | `binding.json` + `worldID`-verified resolution + relink | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0383 | Lock→write→unlock + heartbeat + stale-lock recovery | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0384 | Epoch chain (world/timeline/binding) + resolve endpoint | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0385 | Cached world index → named pending entries; **world-scoped kinds become creatable** (closes AC1 gated half + AC3 faction clause) | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0386 | Object cards (ONE implementation, ten per-kind configurations) | SP-099 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-099.md` |
| T-0387 | Object picker (unfiltered, all worlds) + "Create new…" entry point (disabled → T-0388) | SP-099 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-099.md` |
| T-0388 | In-stack create/edit + edit-state visuals + complete-or-discard | SP-099 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-099.md` |
| T-0389 | Pending presentation + Worlds menu + warning view (**⚠️ includes the unbuilt `offline`/`unmounted` status refinement — AC24**) | SP-102 | EP-031 | 🔵 Backlog | — |
| T-0390 | External Change Repair Matrix — world-package conditions | SP-100 | EP-031 | 🔵 Backlog | — |
| T-0391 | EP-031 verification + Epic close prep | SP-100 | EP-031 | 🔵 Backlog | — |
| T-0392 | `[ScriviCore]` Extend `scrivi.scene.v1`: `tags`, `outline`, `todo` (additive) + `SceneMetaData` + round-trip tests | SP-091 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-091.md` |
| T-0393 | `[ScriviCore]` C ABI: `scrivi_set_scene_tags` / `_outline` / `_todo` + `scrivi_get_scene_notes` + ctest | SP-091 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-091.md` |
| T-0394 | `[ScriviCore]` `scrivi_history_get_tree` — windowed `{aroundNodeID?, maxNodes?}` | SP-092 | EP-030 | 🟠 Implemented - Not Verified | — |
| T-0395 | `[Apple]` `ScriviEngine`/`HistoryCapture` history-tree wrapper + interop | SP-092 | EP-030 | 🟠 Implemented - Not Verified | — |
| T-0396 | `[Apple]` **Typing-session coalescing** — autosave **defers** the commit (records nothing mid-session); entry seals at a real boundary or a **45 s** i… | SP-093 | EP-019 | 🟠 Implemented - Not Verified | — |
| T-0397 | `[Cross]` **Whitespace-kind labels in history** — new `whitespaceKind` tree field (`"newline:2"`) → `whitespaceLabel` (`"⏎ new paragraph"`); `preview`… | SP-093 | EP-019 | 🟠 Implemented - Not Verified | — |
| T-0398 | `[Cross]` **Distinguish added vs. deleted text in history rows** — `minus.circle` glyph + orange tint + "Deleted …" label prefix; `removedLength` carr… | SP-093 | EP-019 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-093.md` |
| T-0399 | `[Apple]` **Card soft-failure isolation** — framework backstop in `CardBodyBoundary` + throwing `makeContent` (default forwards) + failing-card test f… | SP-101 | EP-030 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-101.md` |
| T-0400 | `[ScriviCore]` **History log-segment rotation** — `activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it, s… | — | EP-019 | 🔵 Backlog | — |
| T-0401 | Index rebuild + corruption coverage (missing / corrupt / stale) — Doc 1 AC2 | SP-095 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-095.md` |
| T-0402 | ⚠️ Endpoint-kind resolution via `ObjectIndex` — replaces §5.2's broken ID-prefix rule; amends Doc 1 | SP-096 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-096.md` |
| T-0403 | ⚠️ `FileSystem::createFileExclusive` — exclusive-create primitive Doc 3 §6.5 assumes but that does not exist | SP-097 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-097.md` |
| T-0404 | `rule` relocation to world scope + Package Structure §11 correction (deferred from SP-095) | SP-095 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-095.md` |
| T-0405 | ⚠️ **I-0113** — `worldID` on `scrivi_create/open/delete_object` (breaking ABI widen) | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0406 | `source` object kind (T-0365 ScriviCore half) — **closes EP-031 AC1** | SP-098 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-098.md` |
| T-0407 | ⚠️ **`ScriviEngine` graph + world wrappers** — 14 endpoints (12 planned + `createWorld`/`addWorld`); **+ `detail` on `ScriviError` and the null-result… | SP-099 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-099.md` |
| T-0408 | **Worlds menu** — list / create / bind + picker scope line (new at R4; `listWorlds` had **zero** UI call sites) | SP-099 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-099.md` |
| T-0409 | ⚠️ **`objectKindIsWorldScoped` → all ten kinds world-scoped** (`source` excluded); update `ObjectStore` scope sites + C ABI kind table | SP-103 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-103.md` |
| T-0410 | Worldless-project world-creation prompt | SP-103 | EP-031 | ⛔️ Removed as OBE | — |
| T-0411 | Core + interop test realignment to world scope; **re-verify EP-031 AC1** under the new table | SP-103 | EP-031 | ✅ Verified | ⚠️ *unfiled* — see `../Sprints/Closed/Sprint-SP-103.md` |
| T-0412 | (next available Task ID — unassigned) | — | — | ⚪ Unassigned | — |

---

*Last Updated: 2026-08-16 (consistency audit — **T-0217 corrected from 🔵 Backlog / SP-057 to ✅ Verified
2026-08-11 / SP-094.** It was completed in SP-094 and closed with EP-019; the row had inherited SP-057, the
sprint that was superseded into SP-094. ⚠️ **The 2026-08-15 rebuild took this row's status from
`Task-backlog.md`, which was itself stale** — the one source the rebuild header warns against, used anyway
where no closed-Sprint record was consulted. Counts follow: Verified 368 → **369**, Backlog 25 → **24**,
verified-but-unfiled 179 → **180**. Prior note follows.)*

*2026-08-15 (docs cleanup — index rebuilt from `Verified/`, `Closed/`, `Task-backlog.md` and
every Sprint record; 239 → 411 Task IDs, with 179 verified-but-unfiled Tasks flagged.)*
