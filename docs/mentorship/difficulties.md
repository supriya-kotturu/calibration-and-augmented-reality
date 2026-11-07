# Difficulties log

Raw material for the report (Task 1 asks for limitations/challenges locating the target). One entry per problem: symptom → cause → fix.

| ID | Date | Area | Symptom | Cause | Fix / status |
| --- | --- | --- | --- | --- | --- |
| X-01 | 2026-11-06 | OpenCV 5 API | Most tutorials (incl. the course PDF links) target 3.4/4.x | OpenCV 5 split `calib3d` into `calib` (calibrateCamera), `geometry` (solvePnP, projectPoints) and `stereo`; ArUco/ChArUco live in `objdetect` as `ArucoDetector`/`CharucoDetector`, not the old contrib free functions | Use 5.0 docs (docs.opencv.org/5.0); `opencv2/calib3d.hpp` still exists as an umbrella include |
| X-02 | 2026-11-07 | ChArUco board | (preventive) Boards with an **even** number of rows made by OpenCV < 4.6 or by some online generators use the old layout and fail to detect | OpenCV 4.6 changed the pattern for even row counts; `CharucoBoard::setLegacyPattern` exists for the old one | Generate the board with our own 5.0 code, never download one |
| X-03 | 2026-11-07 | Board display | Square size on the tablet was not measured (no ruler) | — | Accepted: the PNG matches the screen's native 2304×1440 (16:10), so any full-screen view is 1:1 or a uniform scale, and units are squares, so absolute size doesn't matter. If reprojection error is high, check for stretching (e.g. the status bar cropping the view). |
