# Scoreboard

+1 per visible improvement, −1 per repeated hint (see hints.md). Progress update every 6 h of active work.

**Total: 4**

| Date | Δ | Reason |
| --- | --- | --- |
| 2026-11-07 | +1 | Reasoned out the ChArUco calibration advantage (partial boards → corners at the image edges → distortion constrained) |
| 2026-11-07 | +1 | make_board B.1–B.4 correct on the first try: X/Y order right, imwrite result checked; output verified at 2304×1440 |
| 2026-11-07 | +1 | Camera loop T0.1–T0.5: index-vs-URL open, failure exit, resolution logged, waitKey(1) |
| 2026-11-07 | +1 | intrinsics_io: all 5 tests green; missing key told apart from a zero value via FileNode::empty(); cv::Exception on open mapped to the header's runtime_error |
