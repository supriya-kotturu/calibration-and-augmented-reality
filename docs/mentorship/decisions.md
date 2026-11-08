# Decision log

Full format (CV is a new field): options, trade-offs, choice, why the others were rejected.
Owner: **HAND** = you write it, Claude reviews. **SNIPPET** = you type the OpenCV calls from doc pointers; Claude scaffolds plain C++/CMake. **AGENT** = Claude writes it to your spec.

| ID | Date | Decision | Owner | Status |
| --- | --- | --- | --- | --- |
| D-01 | 2026-11-06 | Calibration / AR target | — | decided: ChArUco |
| D-02 | 2026-11-06 | Working mode + ownership | — | decided: mentorship, SNIPPET |
| D-03 | 2026-11-06 | ChArUco board parameters (dictionary, squares, square/marker length) | you | decided: DICT_6X6_250, 9×6, square 1, marker 0.7, borderBits 1, white margin 30 px |
| D-04 | 2026-11-06 | Camera source | you | decided: phone camera streamed over Wi-Fi (URL) |
| D-05 | 2026-11-07 | Commit the generated board PNG | you | decided: commit |
| D-06 | 2026-11-07 | Parallel agent build (amends D-02) | you | decided: C (hybrid) |
| D-07 | 2026-11-07 | World frame | you | decided: +z toward viewer, +y up |
| D-08 | 2026-11-07 | Commit screenshots and calibration captures | you | decided: commit, captures via LFS |

---

## D-01 — Calibration / AR target

**Options**
- A. 9×6 checkerboard (course default): `findChessboardCorners` + `cornerSubPix`.
- B. ArUco grid board: markers only.
- C. ChArUco board (OpenCV 5 `cv::aruco::CharucoBoard` + `CharucoDetector`, in `objdetect`).

**Trade-offs (yours):**
- A's 9×6 corner grid is symmetric, so after a 180° rotation the detector can return the corners in reverse order. That moves the world origin to the opposite corner, and the pose (and any virtual object) flips between frames. In C, every corner gets its ID from the neighbouring marker, so the ordering is fixed.
- A is all-or-nothing: `findChessboardCorners` returns `false` unless it finds every corner in grid order, so a partly covered or partly out-of-frame board gives you nothing.
- Distortion is strongest towards the image corners and edges. A detector that needs the whole board visible keeps every corner away from that region, so the distortion coefficients are poorly constrained. C accepts partial boards, so corners can be collected right up to the image edge.
- Accepted cost of C: markers need more pixels than plain squares (H-02), there are more parameters to choose (D-03), and the 5.0 API has fewer tutorials than the checkerboard (X-01).

**Choice:** C.

## D-02 — Working mode + ownership

Mentorship. You type every OpenCV call, guided by doc pointers; Claude writes the non-OpenCV scaffolding (CMake, arg parsing, loop shape, docs) and reviews. Extensions brainstormed only after Tasks 1–7 basics.

## D-03 — ChArUco board parameters

Decide before Task 1: dictionary, squaresX × squaresY, squareLength, markerLength, and how you display it (print vs screen).

**Dictionary options**
- `DICT_4X4_50`: 4×4 data bits; biggest cells for a given printed size, so it's detectable from farther away and at low resolution. Least room for error correction.
- `DICT_6X6_50`: 6×6 bits; 50 IDs, so the IDs can sit far apart (min Hamming distance is high).
- `DICT_6X6_250` (proposed): same cell size as 6X6_50; 250 IDs packed into the same bit space, so the nearest two IDs are closer together.

**Trade-offs (yours):** a smaller dictionary lets the chosen patterns sit farther apart in the 2^36 space of 6×6 patterns, so a misread is less likely to land on another valid ID. 250 IDs already leave a margin; 50 leave more.
**Choice (2026-11-07):** `DICT_6X6_250`. It keeps 200+ IDs spare, so the multi-target extension can give every board its own IDs, and it matches the OpenCV ChArUco tutorial, which makes troubleshooting easier.
**Accepted cost:** lower min Hamming distance than `DICT_6X6_50`, and smaller cells than 4×4, so the board must be closer to the camera or printed larger. If false IDs or a short detection range show up, log them in difficulties.md and revisit.
**Other parameters (2026-11-07):**
- squareLength = 1 (course units); **markerLength = 0.7** (decided; to be tuned experimentally, results go in difficulties.md).
- Display: Samsung Galaxy Tab S10 FE (10.9", 2304×1440 px, 16:10, ≈ 23.5 × 14.7 cm), paper as fallback. Show the board at native resolution (1 image px = 1 screen px) so the OS doesn't rescale and blur marker edges.
- Grid: **9×6 squares** (decided 2026-11-07): same square size as 7×6 on a 16:10 screen (the rows set it, H-04), and 8×5 = 40 inner corners instead of 30.
- Marker border: **`borderBits = 1`** (decided). It's the minimum: the border is how markers are found, so 0 isn't an option. A thicker border would shrink the data cells and wouldn't help corner accuracy (H-05).
- Board margin: **white, `marginSize = 30` px** (decided) to separate the edge squares from the black bezel. Rendered at the screen's native 2304×1440 → squares ≈ 230 px (min(2244/9, 1380/6)).
- Shared spec: `include/core/board_spec.h` (constants only) so `make_board` and `calibrate` can't disagree.

## D-04 — Camera source

`calibrate <src>`: `<src>` = device index (`0`) or file/stream URL. Calibration is only valid for the camera **and resolution** it was made with.

**Options:** A. laptop webcam (device index); B. phone camera streamed to the laptop by a webcam app (URL); C. phone-recorded video files only.
**Trade-offs:** A is the simplest and lowest-latency, but it's a different camera from the sample video. B uses the same phone as `data/samples/board.mp4`, at the cost of Wi-Fi latency, compression artefacts and dropped frames. C can't be a live demo.
**Choice (2026-11-07):** B. The phone camera is the demo camera.
**Consequences:** calibrate from **the stream itself**, at the exact resolution and orientation the demo uses. The phone's own recorder can crop or stabilise the image differently, so `board.mp4` is for tests and headless runs, not for producing the demo's intrinsics. Fix the streaming app's resolution and orientation, turn off its auto-rotate and stabilisation, and log them in difficulties.md.

## D-05 — Commit the generated board PNG

**Options:** A. commit `data/board/*.png`; B. ignore it and regenerate with `make_board`.
**Trade-offs:** A gives reviewers and the README/report a target to look at without building anything, at the cost of 56 KB and a possible mismatch with `board_spec.h`. B can never be stale, but there's nothing to show without a build.
**Choice:** A. Mitigation: re-run `make_board` and commit the PNG in the same commit as any `board_spec.h` change.

## D-06 — Parallel agent build (amends D-02)

**Options**
- A. Keep D-02 (SNIPPET): you type every OpenCV call; agents only scaffold. Slowest; you learn the most.
- B. Full parallel: Sonnet agents build Phase 0 + WS1–WS4 ([plan](../plan/parallel-build.md)); you review and merge.
- C. Hybrid: you write the OpenCV core in Phase 0 (`detectCharuco`, `calibrateCamera` call, `estimatePose`, `projectPoints` in `draw`); agents build the apps, tests, CMake and docs on top of it in parallel.

**Trade-offs:** B is fastest but the agents write the parts the grade (and the 2-point Q&A) asks you to explain. C keeps the ~150 lines that matter for learning in your hands and still parallelises the rest; cost is that Phase 0 waits on you. A fails the "get this going" goal.
**Choice (2026-11-07):** C. Phase 0 splits: an agent writes headers, stubs, CMake and tests (0a); you write the OpenCV implementations until the tests pass (0b). Apps and docs go to agents in parallel after 0a.

## D-07 — World frame

**Options:** A. OpenCV's ChArUco frame (origin top-left, +y down, so right-handed +z points into the board). B. Course convention (+x right, +y up, +z toward the viewer; next row at (0,-1,0)).
**Trade-offs:** A needs no conversion, but "above the board" becomes −z, which is easy to get backwards while learning. B matches task.md and the lecture material, at the cost of converting OpenCV's corner coordinates in one place.
**Choice:** B, so the code reads the same as the course while the concepts are new. The conversion lives only in `objectPointsFor`; nothing else may use the board's internal corner coordinates.

## D-08 — Commit screenshots and calibration captures

**Options:** A. git-ignore `data/calib/images/`, `data/ar/`, `data/features/` and copy the chosen images into `report/`; B. commit them where the apps write them.
**Trade-offs:** A keeps history small, but every README, docs or report image then has to be copied by hand and can drift from what the app actually produced. B means the docs link straight to real app output, at the cost of repo size (1080×1920 PNGs are ~1–3 MB each, and calibration writes two per save).
**Choice (2026-11-07):** B. They're the evidence for the README, docs and report.
**Mitigation:** the capture PNGs go through Git LFS (`.gitattributes`). The 56 KB board PNG stays a normal git file.
