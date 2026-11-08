# Project 4: Calibration and Augmented Reality

Sai Supriya Kotturu · CS5330 · Code: [github.com/supriya-kotturu/calibration-and-augmented-reality](https://github.com/supriya-kotturu/calibration-and-augmented-reality)

> Sections marked *(drafted by Claude)* were drafted by Claude (Anthropic) from the project's logs and measurements and reviewed by me. Sections marked **TODO (me)** are mine to write.

## Overview

**TODO (me):** project description in my own words (max 200 words).

## Key decisions and trade-offs *(drafted by Claude)*

Full log with options and rejected alternatives: [decisions.md](../docs/mentorship/decisions.md). Architecture: [ARCHITECTURE.md](../ARCHITECTURE.md).

| ID | Decision | Choice and main trade-off |
| --- | --- | --- |
| D-01 | Calibration / AR target | ChArUco over the 9×6 checkerboard: every corner has an ID, so partial views still work and the pose can't flip under a 180° rotation. Cost: markers need more pixels. |
| D-03 | Board parameters | 9×6 squares, `DICT_6X6_250`, marker/square 0.7, shown at the tablet's native 2304×1440 so no rescaling blurs the marker edges. |
| D-04 | Camera source | Phone camera streamed to the laptop by IP Webcam (URL). Calibrated from the stream itself, because intrinsics only hold for one camera, resolution and orientation. |
| D-06 | Who writes what | I hand-wrote the OpenCV core (detection, intrinsics I/O, pose) against tests written first; agents built the apps in parallel. `draw.cpp` moved to Claude at the end to finish on time. |
| D-07 | World frame | Course convention: +x right, +y up, +z toward the viewer. OpenCV's board frame is +y down, so the conversion lives in one function (`objectPointsFor`). |
| D-08 | Captures | Screenshots and calibration views are committed (via Git LFS) as evidence instead of being git-ignored. |

## Task 1: Detect and extract target corners *(drafted by Claude)*

Target: a 9×6 ChArUco board (40 inner corners) displayed on a tablet (D-01). `CharucoDetector::detectBoard` finds the ArUco markers, then interpolates and sub-pixel-refines the chessboard corners between them. Each corner carries an ID (0–39), so a partial board still yields correctly labelled corners.

Limitations and challenges:
- Detection drops out at steep angles and long distances, where the 6×6 marker cells get too few pixels (D-03 trade-off). See the no-pose frame below.
- The tablet screen reflects room lights; a bright glare spot over a marker hides it and the corners around it.
- OpenCV 5 pitfall: with `cv::Mat` outputs `detectBoard` returned 1 corner instead of 40; `std::vector` outputs fixed it (hint H-06).

<img src="../docs/images/task6-ar/live-no-pose-frontal-far.png" width="480" alt="Frame where no pose was found">

## Task 2: Select calibration images *(drafted by Claude)*

`s` saves the last successful detection (at least 6 corners). Because each frame sees a different subset of corners, the 3D points are built per save from that frame's IDs (`objectPointsFor(ids)`) instead of from one fixed `point_set`. Eight views were saved, from straight-on, tilted both ways, grazing, and close up with the board running off the frame edge. The close-up views put corners near the image border, where distortion is strongest.

<img src="../docs/images/task2-calibration/view-003-close-partial-edges-corners.png" width="480" alt="Calibration view with detected ChArUco corners and IDs">

All views: [docs/images/task2-calibration/](../docs/images/task2-calibration/).

## Task 3: Calibrate the camera *(drafted by Claude)*

Stream: IP Webcam at 960×544. Flags: `CALIB_FIX_ASPECT_RATIO`, 5 distortion coefficients.

| | Camera matrix | Distortion (k1, k2, p1, p2, k3) |
| --- | --- | --- |
| Before | fx = fy = 1, (cx, cy) = (480, 272) | 0, 0, 0, 0, 0 |
| After | fx = fy = **690.3**, (cx, cy) = **(477.3, 272.7)** | 0.174, −1.100, −0.0013, −0.0013, 1.746 |

- Reprojection error: **0.38 px** RMS over 7 views ([phone.yml](../data/calib/phone.yml)). A second run during the screen recording reached 0.23 px over 8 views.
- fx = fy is forced by `CALIB_FIX_ASPECT_RATIO` (square pixels); the principal point is within 3 px of the image centre.
- k3 = 1.75 is large for this few views and is probably fitting noise.
- Sample-video check (1080×1920, 15 views, headless): fx = fy = 1748, RMS 0.62 px.

## Task 4: Camera position

`ar` prints `rvec` and `tvec` every frame (board-to-camera: X_cam = R·X_board + tvec). `tvec` is the position of the board origin (corner 0) in the camera frame, in board squares; `tvec[2]` is its distance in front of the camera.

Measured on the sample video: `tvec[2]` stays between about 11.6 and 22 squares, always positive. `rvec` stays near a half-turn about x (|rvec| ≈ 2–2.8 rad), as expected when the board's +z faces the camera.

**TODO (me):** what happened to `tvec[0]` and `rvec` when I slid the phone side to side on the live stream, and whether that makes sense.

<img src="../docs/images/task4-pose/resolution-mismatch-refused.png" width="640" alt="ar refusing intrinsics made at a different resolution">

`ar` also refuses intrinsics made at a different resolution than the stream (above), since the pose would be silently wrong.

## Task 5: Project outside corners and 3D axes *(drafted by Claude)*

The four outer inner-corners (0,0,0), (7,0,0), (7,−4,0) and (0,−4,0) are projected with `projectPoints` and drawn as a yellow outline; the axes are drawn from the origin (X red, Y green, Z blue, 2 squares long). The outline lands on the corner grid and the blue Z axis points out of the board toward the camera, confirming the D-07 frame end to end.

<img src="../docs/images/task6-ar/live-house-top-down-axes-a.png" width="480" alt="Axes and board outline from above">

## Task 6: Virtual object *(drafted by Claude)*

A wireframe house floating one square above the board: white floor, yellow walls, red roof with the ridge along x, a magenta chimney on one roof slope and a cyan door on one face only. It is deliberately asymmetric, so a mirrored or flipped pose would be visible at once. `1` switches to a debug cube.

| | |
| --- | --- |
| <img src="../docs/images/task6-ar/live-house-oblique-axes-a.png" width="360"> | <img src="../docs/images/task6-ar/live-house-steep-side.png" width="360"> |

Videos: [house](../data/videos/ar-live-stream.mp4), [cube and house switching](../data/videos/ar-object-switching-live-stream.mp4).

## Task 7: Robust features *(drafted by Claude)*

`features` shows Harris corners, ORB and SIFT on the live stream (frames downscaled to 960 px), with trackbars for the thresholds.

| Detector | Typical count | Speed | What it responds to |
| --- | --- | --- | --- |
| Harris | 450–1100 corners | ~30–46 fps | Corners inside the marker patterns. The plain chessboard corners score lower; background texture takes over at oblique angles. |
| ORB | capped at 500 | ~30 fps | Bunches on the most contrasted region; at steep angles about half goes to the background. |
| SIFT | 1.2k–2.4k | ~16 fps | Multi-scale: small keypoints on marker bits, large ones spanning whole squares. |

How features could anchor AR: match keypoints between a reference view of a planar target and the live frame, estimate the homography (or, with known 3D positions, the pose via `solvePnP`) from those matches, and draw the object as before. Calibration still comes from the ChArUco board.

The ORB matching screenshots show the catch: the board repeats the same square and similar marker patterns, so many matches cross over to the wrong square. A geometric check such as `findHomography` with RANSAC would be needed to reject them before trusting a pose.

| | |
| --- | --- |
| <img src="../docs/images/task7-features/live-orb-match-stream.png" width="360"> | <img src="../docs/images/task7-features/harris-zoom-single-marker-tuned.png" width="360"> |

All screenshots: [docs/images/task7-features/](../docs/images/task7-features/).

## Results *(drafted by Claude)*

- End-to-end pipeline on a live phone stream: calibration at RMS 0.38 px, then a stable pose and a correctly oriented virtual object from frontal, oblique and top-down views.
- 28 unit tests on a synthetic board pin the world frame, pose recovery (within 2 % in translation and 1° in rotation) and intrinsics I/O.

## Limitations *(drafted by Claude)*

- Few calibration views (7), so k3 is likely overfit; more varied views or `CALIB_FIX_K3` would steady it.
- Detection fails at long range and steep angles, and when screen glare hides a marker.
- Wi-Fi streaming adds latency and occasional dropped frames (`mjpeg overread` warnings).
- The house is a wireframe with no hidden-line removal.

## Future work

- **TODO (me)**, or keep: more calibration views with `CALIB_FIX_K3`; feature-based AR with RANSAC; shaded objects with OpenGL.

## Extensions *(drafted by Claude)*

### 1. AR on pre-captured video

`ar` takes a file as easily as a stream, so the same pipeline inserts the house into the phone's own recording (`data/samples/board.mp4`, 1080×1920, 28 s), using a calibration made from that recording (`sample_video.yml`). The pose was found in 738 of 838 frames; the misses are the frames where the board is too steep or partly out of view.

<img src="../docs/images/extensions/ar-on-recorded-video-frame500.png" width="360" alt="House rendered onto the recorded video">

Video: [ext-ar-on-recorded-video.mp4](../data/videos/ext-ar-on-recorded-video.mp4).

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

**TODO (me):** what I learned.

## Acknowledgements

- OpenCV 5.0 documentation and the ArUco/ChArUco calibration tutorials.
- Claude (Anthropic) wrote the build plan, CMake and test scaffolding, the test suite, `video_source`, `app_args`, `draw.cpp`, the three apps, the architecture docs, the README and the sections of this report marked *(drafted by Claude)*. I made the architecture decisions (D-01 to D-08), wrote the board spec, `make_board`, the camera loop and the core OpenCV code (`charuco.cpp`, `intrinsics_io.cpp`, `pose.cpp`), and wrote the sections marked as mine.

---

## Appendix: Interview prep *(drafted by Claude)*

**Quick overview:** a ChArUco board on a tablet, a phone camera streamed over Wi-Fi, and three C++/OpenCV apps sharing one core library. `calibrate` estimates intrinsics from about 8 views, `ar` estimates the board pose each frame with `solvePnP` and projects a 3D wireframe house, and `features` compares Harris, ORB and SIFT.

1. **Why ChArUco over a checkerboard?** Every corner has an ID, so partial views work (corners reach the image edges, which pins down distortion) and the pose can't flip under a 180° rotation.
2. **Where is the world-frame conversion?** Only in `objectPointsFor`: ID → (col, −row, 0) × square length. OpenCV's own board frame is +y down.
3. **How is `point_list` built when each frame sees different corners?** Per save, from that frame's IDs.
4. **What does the reprojection error tell you?** The average pixel distance between detected corners and corners re-projected through the fitted model. It rises with blur, bad corner detection, too few or too similar views, or the wrong distortion model.
5. **Why must intrinsics match the frame size?** fx, fy, cx and cy are in pixels at the calibrated resolution; a different size scales them. The YAML stores the size and `ar` refuses a mismatch.
