# Calibration and Augmented Reality

CS5330 Project 4: calibrate a camera with a ChArUco board, then use the calibration to place virtual 3D objects on the board in a live video stream. C++17 + OpenCV 5.0. Assignment spec: [task.md](task.md).

> Work in progress. Demo, report and architecture sections are added as tasks land.

## Summary

| Item | Link |
| --- | --- |
| Live demo | TBD |
| Report | [report/report.md](report/report.md) (skeleton) |
| Demo videos | TBD |

## Demos

<!-- Replace each TBD with a ~360 px wide, ~15 s GIF that links to the full MP4 (Git LFS).
<table>
  <tr>
    <td align="center"><a href="TBD-calibration.mp4"><img src="TBD-calibration.gif" width="360" alt="Calibration"></a><br>Calibration</td>
    <td align="center"><a href="TBD-ar-object.mp4"><img src="TBD-ar-object.gif" width="360" alt="AR object"></a><br>AR object</td>
    <td align="center"><a href="TBD-features.mp4"><img src="TBD-features.gif" width="360" alt="Features"></a><br>Features</td>
  </tr>
</table>
-->

## Board

<img src="data/board/charuco_9x6_dict6x6_250.png" alt="9×6 ChArUco board, DICT_6X6_250" width="480">

9×6 ChArUco, `DICT_6X6_250`, square 1 unit, marker 0.7, rendered for a Samsung Tab S10 FE (2304×1440) and shown full-screen at native resolution. Parameters: [include/core/board_spec.h](include/core/board_spec.h); rationale: D-03 in [docs/mentorship/decisions.md](docs/mentorship/decisions.md).

## Architecture

<img src="docs/architecture/overview.png" alt="Architecture overview" width="100%">

Source: [docs/architecture/overview.mmd](docs/architecture/overview.mmd) (Mermaid). Deeper diagrams (calibrate data flow, AR per-frame pipeline, core dependencies, world frame), contracts and test strategy: [ARCHITECTURE.md](ARCHITECTURE.md).

Four executables share one static library, `calibar_core`: `make_board` renders the board, `calibrate` covers Tasks 1-3, `ar` covers Tasks 4-6, `features` covers Task 7. `calibrate` and `ar` talk only through an intrinsics YAML file. Apps other than `make_board` are still being built; see their status in [ARCHITECTURE.md](ARCHITECTURE.md).

## Repo layout

| Path | Contents |
| --- | --- |
| `src/apps/` | One executable per job (`make_board`, `calibrate`, `ar`, `features`) |
| `src/core/`, `include/core/` | `calibar_core` static lib: board spec, ChArUco detection, intrinsics I/O, pose, drawing, video source, CLI args |
| `tests/` | GoogleTest suite with a synthetic ChArUco board |
| `data/board/` | Generated board image (committed as the reference target, D-05) |
| `data/calib/` | Intrinsics YAML (`fixture.yml` is approximate; real calibrations land here) |
| `data/samples/` | Sample video for headless runs (Git LFS) |
| `docs/mentorship/` | Decision log, difficulties log, hints, scoreboard |
| `docs/plan/` | Parallel build plan |
| `docs/architecture/` | Architecture diagram source (`.mmd`) and PNG |
| `report/` | Project report |

## Build and run

Requires Visual Studio 2026 and the prebuilt OpenCV 5.0 at `C:/opencv` (or set `OpenCV_DIR`). Executables land in `out/`; run them from the repo root.

```sh
cmake --preset msvc
cmake --build --preset debug

./out/make_board.exe      # writes data/board/charuco_9x6_dict6x6_250.png
./out/calibrate.exe 0     # camera index, video file, or stream URL
./out/ar.exe 0 --intrinsics data/calib/fixture.yml
./out/features.exe 0

ctest --preset test       # runs the GoogleTest suite
```

The apps share the flags `<source> [--intrinsics path] [--headless] [--max-frames N]` ([include/core/app_args.h](include/core/app_args.h)); `--headless` runs without a window, which is how they are checked on `data/samples/board.mp4`. At the time of writing `ar` and `features` are placeholders and `calibrate` is a bare camera loop, so these commands may not do anything useful until the app branches merge.

## Acknowledgements

- OpenCV documentation and tutorials, including the ArUco/ChArUco calibration tutorials.
- Claude (Anthropic) wrote the scaffolding, tests and app code under the D-06 split. The owner hand-wrote the core OpenCV code in `src/core/` (`charuco`, `intrinsics_io`, `pose`, `draw`); Claude wrote the rest. The owner will finalise this wording.
