# Calibration and Augmented Reality

CS5330 Project 4: calibrate a camera with a ChArUco board, then use the calibration to place virtual 3D objects on the board in a live video stream. C++17 + OpenCV 5.0. Assignment spec: [task.md](task.md).

> Work in progress. Demo, report and architecture sections are added as tasks land.

## Board

<img src="data/board/charuco_9x6_dict6x6_250.png" alt="9×6 ChArUco board, DICT_6X6_250" width="480">

9×6 ChArUco, `DICT_6X6_250`, square 1 unit, marker 0.7, rendered for a Samsung Tab S10 FE (2304×1440) and shown full-screen at native resolution. Parameters: [include/core/board_spec.h](include/core/board_spec.h); rationale: D-03 in [docs/mentorship/decisions.md](docs/mentorship/decisions.md).

## Repo layout

| Path | Contents |
| --- | --- |
| `src/apps/` | One executable per job (`make_board`, `calibrate`) |
| `include/core/` | Shared headers (board spec) |
| `data/board/` | Generated board image (committed as the reference target, D-05) |
| `docs/mentorship/` | Decision log, difficulties log, hints, scoreboard |

## Build and run

Requires Visual Studio 2026 and the prebuilt OpenCV 5.0 at `C:/opencv` (or set `OpenCV_DIR`). Executables land in `out/`; run them from the repo root.

```sh
cmake --preset msvc
cmake --build --preset debug

./out/make_board.exe      # writes data/board/charuco_9x6_dict6x6_250.png
./out/calibrate.exe 0     # camera index, video file, or stream URL
```
