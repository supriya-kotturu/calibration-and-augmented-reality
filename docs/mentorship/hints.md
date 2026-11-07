# Hints given

Same hint needed again → −1 on the scoreboard and `Repeats` +1.

| ID | Date | Topic | Hint (short) | Repeats |
| --- | --- | --- | --- | --- |
| H-01 | 2026-11-06 | ArUco dictionary | Same bit count, fewer IDs → the IDs sit farther apart (higher min Hamming distance) → more bit errors can be corrected and misreads are rarer. Only take a bigger dictionary if you need the IDs (e.g. several boards in one scene). | 0 |
| H-02 | 2026-11-06 | ArUco dictionary | More bits per marker (6×6 vs 4×4) → smaller cells at the same printed size → the marker needs more pixels, so detection fails sooner with distance or low resolution. | 0 |
| H-03 | 2026-11-07 | ChArUco | markerLength / squareLength ≈ 0.7 to start. Too small → the marker bits get tiny and detection range drops. Too large → the white margin shrinks and marker edges bleed into the square corners, which blurs the corner refinement. | 0 |
| H-04 | 2026-11-07 | Board layout | On a fixed display, square size = min(width / squaresX, height / squaresY). Adding squares along the dimension that isn't the limit costs nothing in square size; it only fills empty space. | 0 |
| H-05 | 2026-11-07 | ChArUco | The ChArUco corners you calibrate with are the chessboard corners where two black squares touch, refined to sub-pixel accuracy. Markers only **identify** those corners (which ID is next to which corner). The marker border helps find markers; it does nothing for corner accuracy. | 0 |
| H-06 | 2026-11-07 | ChArUco detection (OpenCV 5) | `CharucoDetector::detectBoard`: pass `std::vector<cv::Point2f>` / `std::vector<int>` as the corner/ID outputs, not `cv::Mat`. With `Mat` outputs a frontal board came back with 1 corner instead of 40. | 0 |
| H-07 | 2026-11-07 | projectPoints | The output point type must match the input precision: `Point3f` in → `std::vector<cv::Point2f>` out (`Point3d` → `Point2d`). A mismatch fails an assertion at runtime, not at compile time. | 0 |
