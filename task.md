# Project 4: Calibration and Augmented Reality

Source: `task.pdf` (Canvas assignment). Points: 30 (2 of which are the post-grading Q&A). Language: **C++ with OpenCV**. Submit `.cpp`/`.h(pp)` files, PDF report, `readme.txt`/`readme.md`.

## Goal

Calibrate a camera, then use the calibration to detect a target and place a virtual object relative to it that moves and orients correctly as the camera or target moves.

## Setup

| Item | Detail |
| --- | --- |
| Target | `checkerboard.png` (Canvas file). Long axis horizontal: **9 columns × 6 rows of internal corners** |
| Display | On a tablet/phone screen, or printed and glued/taped flat (board, notebook) |
| World units | Squares are 1 × 1; define virtual objects in those units |
| Alternative | ArUco boards ([OpenCV tutorial](https://docs.opencv.org/5.0/tutorials/objdetect/aruco_calibration/aruco_calibration.html#calibration-with-aruco-boards)); instructor suggests trying them (may be more stable). OpenCV supports both |
| Same target | Use for both calibration and AR parts |

## Pipeline

```
frame → [1] detect target corners → [2] 's' saves corners + 3D world points
      → [3] calibrateCamera (≥5 frames) → write intrinsics to file
      → [4] read intrinsics; per frame: detect → solvePnP (pose)
      → [5] projectPoints: outside corners / 3D axes → [6] virtual line object
[7] separate program: robust features (Harris / SURF / …) on a custom pattern
```

## Tasks

Each task lists **Implement** (code), **Display** (runtime), and **Report** (deliverable).

### 1. Detect and extract target corners
- **Implement:** detect target and extract corners. Get this working cleanly before Task 2.
  - Checkerboard: `findChessboardCorners`, `cornerSubPix`, `drawChessboardCorners`. Corners as `std::vector<cv::Point2f> corner_set`.
  - ArUco: `cv::aruco::ArucoDetector::detectMarkers`, `drawDetectedMarkers`. Corners as `std::vector<std::vector<cv::Point2f>>` (multiple markers).
  - Docs: [OpenCV camera calibration tutorial](https://docs.opencv.org/master/d4/d94/tutorial_camera_calibration.html).
- **Display:** draw corners on the video when found. Helpful: print number of corners + coordinates of the first corner (checkerboard's first corner is usually upper-left as seen in the image; ArUco is asymmetric so points map to the same target locations regardless of orientation).
- **Report:** which target type is used; limitations/challenges in locating the target.

### 2. Select calibration images
- **Implement:** on a key (e.g. `s`), store the corners from the **last successful** detection into `corner_list` (keep latest image + corners in variables; user may press `s` when the board isn't found).
  - Each save also pushes a `point_set` of 3D world points (identical for every save; world frame is attached to the target).
  - Suggested types:
    ```cpp
    std::vector<cv::Vec3f> point_set;
    std::vector<std::vector<cv::Vec3f>> point_list;
    std::vector<std::vector<cv::Point2f>> corner_list;
    ```
  - Units: mm (measure square size) or squares. Squares: (0,0,0), (1,0,0), (2,0,0), …; with origin upper-left and Z toward the viewer, next row starts at (0,−1,0).
  - `point_set.size() == corner_set.size()`; `point_list.size() == corner_list.size()`.
  - **Most common mistake:** swapping rows and columns in `point_set`.
  - Optionally save the calibration images themselves.
- **Report:** a calibration image with corners highlighted.

### 3. Calibrate the camera
- **Implement:** require **≥ 5** selected frames, then let the user run calibration (or recalibrate continuously after each new frame beyond the minimum, reporting current per-pixel error).
  - `cv::calibrateCamera(point_list, corner_list, imageSize, camera_matrix, dist_coeffs, rvecs, tvecs, …)`; optional per-parameter std-dev outputs.
  - Consider `CALIB_FIX_ASPECT_RATIO` (square pixels).
  - Radial distortion optional (start off); may be needed for phone cameras. Count = length of the zero vector passed: 0 (none, `std::vector<double>` of length 0), 4, 5, or 8 (full).
  - `camera_matrix`: 3×3 `CV_64FC1` (**double**; `<float>` access breaks). Initialize:
    ```
    [1, 0, frame.cols/2]
    [0, 1, frame.rows/2]
    [0, 0, 1          ]
    ```
  - Print camera matrix + distortion coefficients **before and after**, plus final reprojection error. Expect fx ≈ fy, (u0, v0) near image center, error < 1 px (2–3 px OK for large/phone images). Downscaling images is fine for speed.
  - `calibrateCamera` returns per-image rotations/translations; optionally save with the images. OpenCV has a camera-pose visualizer.
  - Let the user write intrinsics (camera matrix + distortion coefficients) to a file.
- **Report:** calibration matrix + reprojection error.

### 4. Calculate current position of the camera
- **Implement:** (separate program allowed, or extend the first.) Read intrinsics from file; video loop: detect target → corners → `solvePnP` → board pose (rotation, translation).
- **Display:** print rotation and translation in real time.
- **Report:** how the values change as the camera (or target) moves side to side; do they make sense?

### 5. Project outside corners or 3D axes
- **Implement:** with the Task 4 pose, `projectPoints` the 3D points of ≥ 4 target corners onto the image in real time; **or** draw 3D axes at the target origin (3 colors helps when building the virtual object).
- **Report:** do reprojected points / axes land in the right places? ≥ 1 image.

### 6. Create a virtual object
- **Implement:** 3D object made of lines, **floating above the board**, **more than one color**. Design in world space as line segments between 3D points (start with 2–3 lines and build up); project endpoints with `projectPoints` (rvec, tvec, distortion, camera matrix); draw lines between projected pairs. Must keep correct orientation as the camera moves.
  - Asymmetric object aids debugging. Aim beyond cube/pyramid/tetrahedron (those are fine for debugging).
- **Report:** describe the object; screenshots and/or videos.

### 7. Detect robust features
- **Implement:** separate program showing features (e.g. Harris corners, SURF) live in a video stream on a pattern of your choice. Experiment with thresholds/settings.
- **Report:** how those feature points could serve as the basis for AR; screenshots and/or videos.

## Extensions

- Especially creative virtual objects/scenes. OpenCV + OpenGL for shaded objects (big extension).
- Multiple targets in the scene (multiple ArUco targets give more options).
- Test several cameras; compare calibrations and result quality.
- AR with a non-checkerboard target (photo, painting, object) with 3–4 easily found, identifiable points; place an AR object relative to it.
- Static images or pre-captured video with targets; insert virtual objects.
- Add a virtual object **and** alter the target so it no longer looks like a target.
- **Uber 1:** recover 3D points of an object in a scene that also contains a calibration target; show the point cloud.
- **Uber 2:** point tracking + AR with SIFT/SURF/ORB features (intrinsics from checkerboard/ArUco).
- **Uber 3:** interface to let someone build a 3D virtual object in the scene.

## Report (PDF, no code)

1. Short project description in your own words (≤ 200 words).
2. Required images, each with a short description.
3. Extensions: description + example images.
4. Short reflection on what you learned.
5. Acknowledgements of materials/people consulted.

### Required report items checklist

| Task | Item |
| --- | --- |
| 1 | Target type; detection limitations/challenges |
| 2 | Calibration image with corners highlighted |
| 3 | Calibration matrix + reprojection error |
| 4 | How rotation/translation change with side-to-side motion; sanity |
| 5 | ≥ 1 image of projected corners/axes; correctness |
| 6 | Virtual object description + screenshots/videos |
| 7 | Features → AR explanation + screenshots/videos |

## Submission

- Gradescope: code (`.cpp`, `.h`/`.hpp`), PDF report, readme (`.txt` or `.md`). Resubmit freely until the deadline.
- Readme must include: name (+ group members), links to videos, time travel days used.
- Deadline policy: on time → max 30/30 (base + extensions); ≤ 1 week late → max 26/30 (no extensions). 8 time travel days per semester, ≤ 3 per assignment, whole days only.
- **Project Q&A:** 10-min session with TA/Professor after grading — 2 of 30 points; be ready to explain implementation and extensions.
