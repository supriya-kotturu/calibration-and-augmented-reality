# Project 4: Calibration and Augmented Reality (report skeleton)

Skeleton only. Every TODO is for the owner; no prose here is final.

## Overview

- TODO: project description in your own words (max 200 words).

## Key decisions and trade-offs

Full log: [decisions.md](../docs/mentorship/decisions.md). Architecture: [ARCHITECTURE.md](../ARCHITECTURE.md), [overview diagram](../docs/architecture/overview.png).

| ID | Title | Owner summary |
| --- | --- | --- |
| D-01 | Calibration / AR target | TODO |
| D-02 | Working mode + ownership | TODO |
| D-03 | ChArUco board parameters | TODO |
| D-04 | Camera source | TODO |
| D-05 | Commit the generated board PNG | TODO |
| D-06 | Parallel agent build | TODO |
| D-07 | World frame | TODO |

## Task 1: Detect and extract target corners

- Required: target type; detection limitations/challenges.
- TODO: target type and why (D-01).
- TODO: limitations/challenges.
- TODO: image placeholder (corners drawn on a frame).

## Task 2: Select calibration images

- Required: a calibration image with corners highlighted.
- TODO: image placeholder.
- TODO: how `point_list` is built per frame for ChArUco.

## Task 3: Calibrate the camera

- Required: calibration matrix + reprojection error.
- TODO: camera matrix, distortion coefficients (before and after), RMS in px, number of frames.
- TODO: is fx ~ fy, is the principal point near the centre.

## Task 4: Camera position

- Required: how rotation/translation change with side-to-side motion; do they make sense?
- TODO: observations.
- TODO: image/plot placeholder.

## Task 5: Project corners or axes

- Required: at least one image of projected corners/axes; do they land correctly?
- TODO: image placeholder.
- TODO: correctness assessment.

## Task 6: Virtual object

- Required: object description + screenshots/videos.
- TODO: description of the object and its colours.
- TODO: screenshots / video links.

## Task 7: Robust features

- Required: how features could be the basis for AR; screenshots/videos.
- TODO: Harris and ORB observations, parameter effects.
- TODO: how features could anchor AR.
- TODO: screenshots / video links.

## Results

- TODO

## Limitations

- TODO

## Future work

- TODO

## Extensions

- TODO: description + example images, or "none".

## Reflection

- TODO: what you learned (your voice).

## Acknowledgements

- TODO: OpenCV docs/tutorials, people consulted.
- TODO: state exactly what Claude (Anthropic) wrote vs what the owner hand-wrote (see README).

---

## Appendix: Interview prep

### Quick overview

- TODO: 3-4 sentences on what was built.

### Likely questions

1. TODO: Why ChArUco over a checkerboard? (D-01)
2. TODO: Why is the world frame +z toward the viewer, and where is the conversion? (D-07)
3. TODO: How is `point_list` built when each frame sees different corners?
4. TODO: What does the reprojection error tell you, and what would raise it?
5. TODO: Why must intrinsics match the live frame size, and how is that enforced? (D-04)
