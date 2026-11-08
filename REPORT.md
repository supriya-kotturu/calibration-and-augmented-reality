# Project 4: Calibration and Augmented Reality

Sai Supriya Kotturu · CS5330 · Code: [github.com/supriya-kotturu/calibration-and-augmented-reality](https://github.com/supriya-kotturu/calibration-and-augmented-reality) · PDF: [REPORT.pdf](REPORT.pdf)

## Overview

This project is about learning how to calibrate a camera and use that calibration to place virtual objects in a scene. I calibrated my phone's camera with a ChArUco board shown on a tablet, then estimated the board's pose in every frame and drew 3D objects on it. The end result shows how the ChArUco board helps identify the corners on the board, even when only part of it is in the frame. I was able to render a cube and a house on the board live, using the IP Webcam app on my phone to stream its camera feed to my laptop. A separate program compares Harris, ORB and SIFT features on the same stream.

## Key decisions and trade-offs

Full log with options and rejected alternatives: [decisions.md](docs/mentorship/decisions.md). Architecture: [ARCHITECTURE.md](ARCHITECTURE.md).

| ID | Decision | Choice and main trade-off |
| --- | --- | --- |
| D-01 | Calibration / AR target | ChArUco over the 9×6 checkerboard: every corner has an ID, so partial views still work and the pose can't flip under a 180° rotation. Cost: markers need more pixels. |
| D-03 | Board parameters | 9×6 squares, `DICT_6X6_250`, marker/square 0.7, shown at the tablet's native 2304×1440 so no rescaling blurs the marker edges. |
| D-04 | Camera source | Phone camera streamed to the laptop by IP Webcam (URL). Calibrated from the stream itself, because intrinsics only hold for one camera, resolution and orientation. |
| D-06 | Who writes what | I hand-wrote the OpenCV core (detection, intrinsics I/O, pose) against tests written first, while Claude built the apps in parallel. See [AI usage](#ai-usage). |
| D-07 | World frame | Course convention: +x right, +y up, +z toward the viewer. OpenCV's board frame is +y down, so the conversion lives in one function (`objectPointsFor`). |
| D-08 | Captures | Screenshots and calibration views are committed (via Git LFS) as evidence instead of being git-ignored. |

## Task 1: Detect and extract target corners

Target: a 9×6 ChArUco board (40 inner corners) displayed on a tablet (D-01). `CharucoDetector::detectBoard` finds the ArUco markers, then interpolates and sub-pixel-refines the chessboard corners between them. Each corner carries an ID (0–39), so a partial board still yields correctly labelled corners.

Limitations and challenges:
- Detection drops out at steep angles and long distances, where the 6×6 marker cells get too few pixels (D-03 trade-off).
- Motion blur defeats it even on an ideal view (third image below). That frame is frontal and close, but the phone was moving when it was captured: every marker edge is doubled, the 6×6 bit patterns smear and stop decoding, and with too few identified markers there are fewer than the 6 corners `ar` needs for a pose (the HUD shows "no pose"). IP Webcam's JPEG compression softens the edges further. Holding the phone still for a moment, or using more light so the exposure is shorter, avoids it.
- The tablet screen reflects room lights; a bright glare spot over a marker hides it and the corners around it.
- OpenCV 5 pitfall: with `cv::Mat` outputs `detectBoard` returned 1 corner instead of 40; `std::vector` outputs fixed it (hint H-06).

<table>
  <tr><td align="center" valign="top"><img src="docs/images/task2-calibration/view-001-frontal-close-corners.png" width="240" alt="All 40 corners detected"><br><sub>Frontal: all 40 corners found, each drawn with its ID</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-005-grazing-partial-corners.png" width="240" alt="Partial board at a grazing angle"><br><sub>Grazing angle with part of the board out of view: the visible corners are still found and correctly labelled</sub></td><td align="center" valign="top"><img src="docs/images/task6-ar/live-no-pose-frontal-far.png" width="240" alt="Motion-blurred frame with no detection"><br><sub>Similar frontal view with motion blur: the marker bits smear and too few markers decode for a pose</sub></td></tr>
</table>

## Task 2: Select calibration images

`s` saves the last successful detection (at least 6 corners). Because each frame sees a different subset of corners, the 3D points are built per save from that frame's IDs (`objectPointsFor(ids)`) instead of from one fixed `point_set`. I saved eight views: straight-on, tilted both ways, grazing, and close up with the board running off the frame edge. The close-up views put corners near the image border, where distortion is strongest.

The eight calibration views, with detected corners and their IDs drawn on:

<table>
  <tr><td align="center" valign="top"><img src="docs/images/task2-calibration/view-001-frontal-close-corners.png" width="180" alt="1. Frontal, close"><br><sub>1. Frontal, close</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-002-frontal-far-corners.png" width="180" alt="2. Frontal, far"><br><sub>2. Frontal, far</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-003-close-partial-edges-corners.png" width="180" alt="3. Close, board past the frame edge"><br><sub>3. Close, board past the frame edge</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-004-tilted-left-oblique-corners.png" width="180" alt="4. Tilted left"><br><sub>4. Tilted left</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task2-calibration/view-005-grazing-partial-corners.png" width="180" alt="5. Grazing angle, partial"><br><sub>5. Grazing angle, partial</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-006-frontal-mid-corners.png" width="180" alt="6. Frontal, mid distance"><br><sub>6. Frontal, mid distance</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-007-steep-far-corners.png" width="180" alt="7. Steep and far"><br><sub>7. Steep and far</sub></td><td align="center" valign="top"><img src="docs/images/task2-calibration/view-008-tilted-right-corners.png" width="180" alt="8. Tilted right"><br><sub>8. Tilted right</sub></td></tr>
</table>

## Task 3: Calibrate the camera

Stream: IP Webcam at 960×544. Flags: `CALIB_FIX_ASPECT_RATIO`, 5 distortion coefficients.

| | Camera matrix | Distortion (k1, k2, p1, p2, k3) |
| --- | --- | --- |
| Before | fx = fy = 1, (cx, cy) = (480, 272) | 0, 0, 0, 0, 0 |
| After | fx = fy = **690.3**, (cx, cy) = **(477.3, 272.7)** | 0.174, −1.100, −0.0013, −0.0013, 1.746 |

- Reprojection error: **0.38 px** RMS over 7 views ([phone.yml](data/calib/phone.yml)). A second run during the screen recording reached 0.23 px over 8 views.
- fx = fy is forced by `CALIB_FIX_ASPECT_RATIO` (square pixels); the principal point is within 3 px of the image centre.
- k3 = 1.75 is large for this few views and is probably fitting noise.
- Sample-video check (1080×1920, 15 views, headless): fx = fy = 1748, RMS 0.62 px.

## Task 4: Camera position

`ar` prints `rvec` and `tvec` every frame (board-to-camera: X_cam = R·X_board + tvec). `tvec` is the position of the board origin (corner 0) in the camera frame, in board squares; `tvec[2]` is its distance in front of the camera.

Across the sample video, `tvec[2]` stays between about 11.6 and 22 squares and is always positive (the board is always in front of the camera). `rvec` stays near a half-turn about x (|rvec| ≈ 2–2.8 rad), as expected when the board's +z faces the camera.

Sliding the camera sideways mostly changes `tvec[0]`: the board origin moves across the camera's x axis, so if the camera moves right, the origin ends up further left in camera coordinates and `tvec[0]` decreases. `tvec[2]` (distance) and `rvec` (the board's orientation relative to the camera) stay roughly constant for a pure sideways slide, and only change when the camera also tilts or moves closer. The sample video shows this, mixed with other motion: between frames 500 and 700 the camera swings across the board and also pulls back, so `tvec[0]` goes from −3.5 to −0.8 squares while `tvec[2]` grows from 11.7 to 21.9. Through all of it, the x-component of `rvec` stays near the same half-turn (2.03 → 2.09 rad). The values make sense: the units are board squares, and the signs follow the camera's x-right, y-down, z-forward convention.

<img src="docs/images/task4-pose/resolution-mismatch-refused.png" width="640" alt="ar refusing intrinsics made at a different resolution">

`ar` also refuses intrinsics made at a different resolution than the stream (above), since the pose would be silently wrong.

## Task 5: Project outside corners and 3D axes

The four outer inner-corners (0,0,0), (7,0,0), (7,−4,0) and (0,−4,0) are projected with `projectPoints` and drawn as a yellow outline; the axes are drawn from the origin (X red, Y green, Z blue, 2 squares long). The outline lands on the corner grid and the blue Z axis points out of the board toward the camera, confirming the D-07 frame end to end.

<img src="docs/images/task6-ar/live-house-top-down-axes-a.png" width="480" alt="Axes and board outline from above">

## Task 6: Virtual object

The virtual object is a wireframe house floating one square above the board: white floor, yellow walls, red roof with the ridge along x, a magenta chimney on one roof slope and a cyan door on one face only. I made it deliberately asymmetric so a mirrored or flipped pose would be obvious at once. Pressing `1` switches to a debug cube.

<table>
  <tr><td align="center" valign="top"><img src="docs/images/task6-ar/live-house-oblique-axes-a.png" width="240" alt="Low oblique view, with axes"><br><sub>Low oblique view, with axes</sub></td><td align="center" valign="top"><img src="docs/images/task6-ar/live-house-oblique-axes-b.png" width="240" alt="Oblique, camera moved"><br><sub>Oblique, camera moved</sub></td><td align="center" valign="top"><img src="docs/images/task6-ar/live-house-steep-side.png" width="240" alt="Steep side view"><br><sub>Steep side view</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task6-ar/live-house-top-down-axes-a.png" width="240" alt="Top-down: X red, Y green, Z blue"><br><sub>Top-down: X red, Y green, Z blue</sub></td><td align="center" valign="top"><img src="docs/images/task6-ar/live-house-top-down-axes-b.png" width="240" alt="Top-down, rotated"><br><sub>Top-down, rotated</sub></td><td align="center" valign="top"><img src="docs/images/task6-ar/house-above-board-axes-sample-video.png" width="240" alt="Recorded video (portrait)"><br><sub>Recorded video (portrait)</sub></td></tr>
</table>

Videos: [house](data/videos/ar-live-stream.mp4), [cube and house switching](data/videos/ar-object-switching-live-stream.mp4).

## Task 7: Robust features

`features` shows Harris corners, ORB and SIFT on the live stream (frames downscaled to 960 px), with trackbars for the thresholds.

| Detector | Typical count | Speed | What it responds to |
| --- | --- | --- | --- |
| Harris | 450–1100 corners | ~30–46 fps | Corners inside the marker patterns. The plain chessboard corners score lower; background texture takes over at oblique angles. |
| ORB | capped at 500 | ~30 fps | Bunches on the most contrasted region; at steep angles about half goes to the background. |
| SIFT | 1.2k–2.4k | ~16 fps | Multi-scale: small keypoints on marker bits, large ones spanning whole squares. |

How features could anchor AR: match keypoints between a reference view of a planar target and the live frame, estimate the homography (or, with known 3D positions, the pose via `solvePnP`) from those matches, and draw the object as before. Calibration still comes from the ChArUco board.

The ORB matching screenshots show the catch: the board repeats the same square and similar marker patterns, so many matches cross over to the wrong square. A geometric check such as `findHomography` with RANSAC would be needed to reject them before trusting a pose.

Detectors across viewpoints (rows: Harris, ORB, SIFT) and close-ups of a single marker (last row):

<table>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/live-harris-frontal.png" width="240" alt="Harris, frontal: 1084 corners"><br><sub>Harris, frontal: 1084 corners</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-harris-tilted.png" width="240" alt="Harris, tilted: 570"><br><sub>Harris, tilted: 570</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-harris-oblique-background-clutter.png" width="240" alt="Harris, oblique: background takes over"><br><sub>Harris, oblique: background takes over</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/live-orb-500-frontal-spread.png" width="240" alt="ORB, frontal: 500 (capped)"><br><sub>ORB, frontal: 500 (capped)</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-orb-500-frontal-clustered.png" width="240" alt="ORB, frontal: clustered"><br><sub>ORB, frontal: clustered</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-orb-500-steep-background.png" width="240" alt="ORB, steep: half on background"><br><sub>ORB, steep: half on background</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/live-sift-frontal.png" width="240" alt="SIFT, frontal: 1980"><br><sub>SIFT, frontal: 1980</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-sift-grazing-angle.png" width="240" alt="SIFT, grazing: 2119"><br><sub>SIFT, grazing: 2119</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/live-sift-tilted-background-texture.png" width="240" alt="SIFT, tilted: 2408 incl. blanket"><br><sub>SIFT, tilted: 2408 incl. blanket</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/harris-zoom-single-marker-tuned.png" width="240" alt="Harris on one marker (tuned)"><br><sub>Harris on one marker (tuned)</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/orb-zoom-single-marker.png" width="240" alt="ORB on one marker: 52"><br><sub>ORB on one marker: 52</sub></td><td align="center" valign="top"><img src="docs/images/task7-features/sift-zoom-single-marker.png" width="240" alt="SIFT on one marker: 89"><br><sub>SIFT on one marker: 89</sub></td></tr>
</table>

ORB matching against a reference frame (reference on the right). Many lines cross to the wrong square because the pattern repeats:

<table>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/live-orb-match-stream.png" width="600" alt="Live stream: 66 good matches"><br><sub>Live stream: 66 good matches</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/orb-match-partial-board-a.png" width="600" alt="Partial board: ~95 matches, many crossing"><br><sub>Partial board: ~95 matches, many crossing</sub></td></tr>
  <tr><td align="center" valign="top"><img src="docs/images/task7-features/orb-match-dim-vs-lit-a.png" width="600" alt="Dim live vs lit reference: 60"><br><sub>Dim live vs lit reference: 60</sub></td></tr>
</table>

## Results

- End-to-end pipeline on a live phone stream: calibration at RMS 0.38 px, then a stable pose and a correctly oriented virtual object from frontal, oblique and top-down views.
- 28 unit tests on a synthetic board pin the world frame, pose recovery (within 2 % in translation and 1° in rotation) and intrinsics I/O.

## Limitations

- Few calibration views (7), so k3 is likely overfit; more varied views or `CALIB_FIX_K3` would steady it.
- Detection fails at long range and steep angles, when screen glare hides a marker, and under motion blur. See the motion-blur comparison in Task 1.
- Wi-Fi streaming adds latency and occasional dropped frames (`mjpeg overread` warnings).
- The house is a wireframe with no hidden-line removal.

## Future work

This project can be extended with:

- more calibration views, with `CALIB_FIX_K3` to keep the third distortion term from overfitting;
- feature-based AR, with RANSAC to reject the wrong matches caused by the board's repeating pattern;
- shaded objects rendered with OpenGL instead of wireframes.

## Extensions

### 1. AR on pre-captured video

`ar` takes a file as easily as a stream, so the same pipeline inserts the house into the phone's own recording (`data/samples/board.mp4`, 1080×1920, 28 s), using a calibration made from that recording (`sample_video.yml`). The pose was found in 738 of 838 frames; the misses are the frames where the board is too steep or partly out of view.

<img src="docs/images/extensions/ar-on-recorded-video-frame500.png" width="360" alt="House rendered onto the recorded video">

Video: [ext-ar-on-recorded-video.mp4](data/videos/ext-ar-on-recorded-video.mp4).

### 2. Comparing two camera modes of the same phone

The same phone camera, calibrated in two modes:

| | IP Webcam stream (live demo) | Phone camera app recording |
| --- | --- | --- |
| Resolution | 960×544, landscape | 1080×1920, portrait |
| Views | 7, chosen by hand | 15, auto-picked headless (every 15th detection) |
| fx = fy (px) | 690.3 | 1748.1 |
| Principal point vs centre | (477.3, 272.7) vs (480, 272) | (585.7, 878.2) vs (540, 960) |
| k1, k2, k3 | 0.174, −1.100, 1.746 | 0.043, −0.755, 2.211 |
| RMS | **0.38 px** | 0.62 px |
| Field of view, long side | 2·atan(480 / 690.3) ≈ **69.6°** | 2·atan(960 / 1748.1) ≈ **57.5°** |

- **Focal length doesn't just scale with resolution.** Going from 960 to 1920 px on the long side should double fx (690 → 1380), but the recording gives 1748. The recorder's field of view is about 12° narrower, consistent with the camera app cropping the sensor for video (e.g. stabilisation).
- **Why this matters (D-04):** a calibration from the camera app's recordings would be wrong for the live stream even at the same pixel count. The live demo had to be calibrated from the stream itself.
- **Hand-picked views beat auto-picked ones.** The recording's principal point is 82 px off centre vertically despite more views. Its auto-picked frames came from one continuous sweep and are less varied than the 7 deliberate views (tilted both ways, close-ups at the frame edge).

## Reflection

This project gave me an opportunity to explore the latest OpenCV (5.0) documentation and APIs. I learned that calibration doesn't depend on the outer corners of the checkerboard: the inner corners, where two black squares meet, are the points that get IDs and are measured. The checkerboard is a good starting point, but it has drawbacks. If the board is rotated 180°, we can't tell one corner from the other, so the pose can flip. That's where ArUco markers help: they are asymmetric, unique markers that identify each corner on the board. A ChArUco board is just a combination of the two, a chessboard with ArUco markers in its white squares.

## AI usage

I used Claude (Anthropic), through Claude Code, as a mentor and as a coding agent.

**What I did**

- Made every design decision (D-01 to D-08 in the [decision log](docs/mentorship/decisions.md)): ChArUco over the checkerboard, the board parameters, the phone stream as the camera, the world frame, and how the work was split.
- Wrote the board spec, `make_board` and the first camera loop.
- Hand-wrote the core OpenCV code against tests: `charuco.cpp` (detection, corner ID → world point), `intrinsics_io.cpp` (save/load) and `pose.cpp` (`solvePnP`).
- Ran every live session: calibrated the phone stream, tested the AR app and the feature detectors, and took all the screenshots and recordings.
- Wrote the Overview, Future work and Reflection, and reviewed everything else.

**What Claude did**

- Mentored the core code with questions and hints (logged in [hints.md](docs/mentorship/hints.md)), reviewed my code and rewrote its comments.
- Wrote the build plan, the CMake setup, the GoogleTest suite and the small shared helpers (`video_source`, `app_args`).
- Wrote `draw.cpp` and the three apps (`calibrate`, `ar`, `features`), using parallel sub-agents, and fixed the bugs found in live testing.
- Wrote the README and architecture docs, converted my recordings into the demo videos and GIFs, rendered the extension video, and drafted the factual sections of this report (Key decisions, Tasks 1–7, Results, Limitations, Extensions) from the logs and measurements.
- Edited my sections for grammar and clarity.

## Acknowledgements

- OpenCV 5.0 documentation and the ArUco/ChArUco calibration tutorials.
