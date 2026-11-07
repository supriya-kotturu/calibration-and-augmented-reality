# Decision log

Full format (CV is a new field): options, trade-offs, choice, why the others were rejected.
Owner: **HAND** = you write it, Claude reviews. **SNIPPET** = you type the OpenCV calls from doc pointers; Claude scaffolds plain C++/CMake. **AGENT** = Claude writes it to your spec.

| ID | Date | Decision | Owner | Status |
| --- | --- | --- | --- | --- |
| D-01 | 2026-11-06 | Calibration / AR target | — | decided: ChArUco |
| D-02 | 2026-11-06 | Working mode + ownership | — | decided: mentorship, SNIPPET |
| D-03 | 2026-11-06 | ChArUco board parameters (dictionary, squares, square/marker length) | you | decided: DICT_6X6_250, 9×6, square 1, marker 0.7, borderBits 1, white margin 30 px |
| D-04 | 2026-11-06 | Camera source | you | open: exe takes index or URL |
| D-05 | 2026-11-07 | Commit the generated board PNG | you | decided: commit |

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

Open. `calibrate <src>`: `<src>` = device index (`0`) or file/stream URL. Calibration is only valid for the camera **and resolution** it was made with.

## D-05 — Commit the generated board PNG

**Options:** A. commit `data/board/*.png`; B. ignore it and regenerate with `make_board`.
**Trade-offs:** A gives reviewers and the README/report a target to look at without building anything, at the cost of 56 KB and a possible mismatch with `board_spec.h`. B can never be stale, but there's nothing to show without a build.
**Choice:** A. Mitigation: re-run `make_board` and commit the PNG in the same commit as any `board_spec.h` change.
