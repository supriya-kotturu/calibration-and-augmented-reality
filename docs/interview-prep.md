# Interview prep: Calibration and Augmented Reality

**Quick overview:** a ChArUco board on a tablet, a phone camera streamed over Wi-Fi, and three C++/OpenCV apps sharing one core library. `calibrate` estimates intrinsics from about 8 views, `ar` estimates the board pose each frame with `solvePnP` and projects a 3D wireframe house, and `features` compares Harris, ORB and SIFT.

1. **Why ChArUco over a checkerboard?** Every corner has an ID, so partial views work (corners reach the image edges, which pins down distortion) and the pose can't flip under a 180° rotation.
2. **Where is the world-frame conversion?** Only in `objectPointsFor`: ID → (col, −row, 0) × square length. OpenCV's own board frame is +y down.
3. **How is `point_list` built when each frame sees different corners?** Per save, from that frame's IDs.
4. **What does the reprojection error tell you?** The average pixel distance between detected corners and corners re-projected through the fitted model. It rises with blur, bad corner detection, too few or too similar views, or the wrong distortion model.
5. **Why must intrinsics match the frame size?** fx, fy, cx and cy are in pixels at the calibrated resolution; a different size scales them. The YAML stores the size and `ar` refuses a mismatch.
