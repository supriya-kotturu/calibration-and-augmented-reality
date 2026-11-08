# Interview prep: Calibration and Augmented Reality

For the 10-minute post-grading Q&A (2 of 30 points). Read the **answer** aloud and check you could say it without looking; the *follow-up* is what a grader asks next.

## 60-second pitch

I calibrated my phone's camera with a ChArUco board shown on a tablet, streamed over Wi-Fi with IP Webcam. Then I used that calibration to estimate the board's pose in every frame and draw a 3D wireframe house floating above it. Three C++/OpenCV 5 apps share one core library:
- `calibrate`: I save about 8 views and run `calibrateCamera`. I got an RMS of 0.38 px at 960×544.
- `ar`: per frame, detect, then `solvePnP`, then `projectPoints`, then draw.
- `features`: compares Harris, ORB and SIFT live.

I hand-wrote the core: corner detection, the corner-ID → world-point mapping, intrinsics save/load and pose. 28 tests on a synthetic board pin the world frame and pose accuracy.

## Numbers to have ready

| What | Value |
| --- | --- |
| Board | 9×6 squares → 8×5 = **40 inner corners**, `DICT_6X6_250`, marker/square 0.7 |
| Stream | IP Webcam, **960×544** |
| Calibration | fx = fy = **690.3 px**, principal point (477.3, 272.7) vs centre (480, 272), **RMS 0.38 px**, 7 views |
| Distortion | k1 0.174, k2 −1.100, k3 1.746 (k3 likely overfit) |
| Sample recording | 1080×1920, fx = 1748, RMS 0.62 px, FOV 57.5° vs 69.6° on the stream |
| AR on recording | pose in **738 / 838** frames |
| Tests | 28; pose recovered within 2 % translation, 1° rotation |

## Concepts

1. **What does calibration estimate?**
   **Answer:** the intrinsics:
   - focal lengths fx, fy, in pixels;
   - the principal point cx, cy, where the optical axis hits the sensor;
   - lens distortion k1, k2, p1, p2, k3.

   Together they map a 3D direction to a pixel. `calibrateCamera` also returns a pose for each view (the extrinsics), but those are thrown away.
   *Follow-up: why do you need several views?* One planar view can't separate focal length from distance. Views at different tilts constrain fx, fy and the centre independently.

2. **Why ChArUco over a checkerboard?**
   **Answer:** every corner has an ID from the neighbouring marker. That gives two things:
   - partial views work, so corners can reach the image edges, where distortion is strongest;
   - the corner order can't flip under a 180° rotation, which would flip the pose.

   The cost is that markers need more pixels than plain squares, so detection fails earlier with distance.
   *Follow-up: which points are actually measured?* The chessboard corners where two black squares meet, refined to sub-pixel accuracy. The markers only identify them.

3. **What does the reprojection error mean?**
   **Answer:** it's the RMS pixel distance between each detected corner and where the fitted model re-projects its 3D point. Mine is 0.38 px. It rises with blur, poor corner detection, too few or too similar views, or the wrong distortion model.
   *Follow-up: is lower always better?* No. Few views plus 5 distortion terms can overfit, giving a low RMS with a wild k3. Held-out views, or `CALIB_FIX_K3`, would test that.

4. **Why `CALIB_FIX_ASPECT_RATIO`?**
   **Answer:** it assumes square pixels (fx = fy), which is true of phone sensors. That's one less parameter to fit from few views.
   *Follow-up: what if the stream were stretched?* For example, 720×480 from a 4:3 sensor. Then the pixels aren't square, the fit gets worse and the RMS rises. That's why I avoided that resolution.

5. **What do `rvec` and `tvec` mean?**
   **Answer:** they define board → camera, with X_cam = R(rvec)·X_board + tvec.
   - `tvec` is where corner 0 sits in camera coordinates, in board squares.
   - `tvec[2]` is the distance in front of the camera.
   - `rvec` is a Rodrigues vector: its direction is the rotation axis and its length is the angle in radians.

   *Follow-up: what changes when you slide sideways?* Mostly `tvec[0]`. `rvec` and `tvec[2]` stay about the same unless you also tilt or move closer.

6. **How does `solvePnP` work, in one breath?**
   **Answer:** given the known 3D points and their 2D detections, it finds the rotation and translation that minimise reprojection error. It undistorts with the intrinsics, then solves. `SOLVEPNP_ITERATIVE` refines with Levenberg–Marquardt. It needs ≥ 4 points; I require 6, because a few nearly collinear corners give a jumpy pose.

7. **Why must the intrinsics match the frame size?**
   **Answer:** fx, fy, cx and cy are in pixels at the calibrated resolution, so a different size scales them. The YAML stores the size, and `ar` exits with a mismatch message instead of drawing a silently wrong pose.

## My code (D-06: I wrote these)

8. **`objectPointsFor`: how does an ID become a 3D point?**
   **Answer:** `col = id % 8`, `row = id / 8`, giving the point `(col·L, −row·L, 0)`. The −row is because D-07 makes +y up, while OpenCV's board frame has +y down. This function is the only place that conversion happens, so calibration and pose can't disagree.
   *Follow-up: why not use `board.getChessboardCorners()`?* That's OpenCV's +y-down frame. "Above the board" would become −z.

9. **`detectCharuco`: anything surprising?**
   **Answer:** in OpenCV 5, `detectBoard` with `cv::Mat` outputs returned 1 corner instead of 40; `std::vector` outputs fixed it (H-06). The detector is a function-local `static`, so it's built once, not every frame.

10. **`loadIntrinsics`: how do you tell a missing key from a zero?**
    **Answer:** `fs[key].empty()`. Reading a missing node with `>>` silently leaves the default 0, which would load a half-written file as valid. So it checks every key first, and returns `nullopt` if any is missing.

11. **`estimatePose`: what did you get wrong first?**
    **Answer:** two things.
    - I passed `markerCorners` instead of the ChArUco `corners`. Markers only identify corners; the corners themselves are what pair with `objectPointsFor(ids)`.
    - I passed calibration statistics where `solvePnP` expects the camera matrix and distortion.

    The tests caught both.

12. **A bug you hit and how you found it?**
    **Answer:** in `objectPointsFor`, the `return` was inside the loop, so it returned after one point, and an empty input fell off the end of the function (undefined behaviour). Three tests failed with exact expected-vs-actual values, which pointed straight at it. Another one: a test passed one run and failed the next because a `cv::Mat` wrapped a temporary `std::vector` without copying it.

13. **How did you test without a camera?**
    **Answer:** a synthetic board. Render it, warp it with a known K and pose, detect it, then check the recovered pose is within 2 % and 1°. One test checks that a point at +z projects closer to the camera, which proves the D-07 frame end to end.

## Design decisions

14. **D-04: why calibrate from the stream and not the phone's camera app?**
    **Answer:** the recording's field of view is 12° narrower, because the camera app crops the sensor for video. So its fx is 1748, not the 1380 that resolution scaling alone predicts. A calibration from it would be wrong for the stream even at the same pixel count.

15. **D-07: why +z toward the viewer?**
    **Answer:** it matches the course convention, so "above the board" is +z and the house sits at z > 0. The cost is one conversion, isolated in `objectPointsFor`.

16. **D-06: how did you split the work with AI?**
    **Answer:** I made every decision and hand-wrote the OpenCV core (detection, I/O, pose) against tests written first. Claude wrote the scaffolding, tests, apps and docs, and mentored the core with hints. `draw.cpp` moved to Claude at the end to finish on time. It's all logged in the decision log and the AI usage section.

## Task 7 and extensions

17. **How could features replace the board for AR?**
    **Answer:** match keypoints between a reference view of a planar target and the live frame. Then estimate a homography, or a pose with `solvePnP` if the 3D positions are known, and draw as before. The catch is visible in my ORB shots: the board's repeating pattern produces wrong matches, so RANSAC (`findHomography`) is needed to reject them.
    *Follow-up: Harris vs ORB vs SIFT?* Harris is fast but only finds corners, with no descriptor. ORB is a fast binary descriptor, capped at 500. SIFT is scale-invariant and the most features, but half ORB's speed (about 16 vs 30 fps).

18. **Why did a frontal, close frame give no pose?**
    **Answer:** motion blur. The marker bit patterns smeared, too few decoded, and I got fewer than 6 corners. More light (a shorter exposure) or holding still fixes it.

19. **Your extensions?**
    **Answer:**
    - AR on a pre-captured video: the same pipeline, file input, pose in 738 of 838 frames.
    - Comparing two camera modes of the same phone: this is what showed the camera app crops for video.

## Curveballs

- **"What would you change with more time?"** More varied calibration views plus `CALIB_FIX_K3`; RANSAC-filtered feature AR; shaded rendering with hidden-surface removal.
- **"Where would this break in production?"**
  - Lighting and glare on the tablet.
  - Motion blur.
  - Wi-Fi latency and dropped frames.
  - Any zoom or resolution change invalidating the calibration.
- **"Why is k3 so large?"** 7 views can't constrain a sixth-order radial term well. It absorbs noise, while the RMS still looks good.
