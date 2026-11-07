/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Projection drawing for axes and 3D segments. Owner implements by hand (D-06); stub
           until then.
*/
#include "core/draw.h"

namespace calibar {

void drawAxes(cv::Mat& /*img*/, const Pose& /*pose*/, const Intrinsics& /*intr*/,
              float /*length*/) {
    // TODO(owner, D-06): project origin + 3 axis tips with cv::projectPoints (H-07), then draw
    // them in the colours draw.h specifies.
}

void drawSegments(cv::Mat& /*img*/, const std::vector<Segment3D>& /*segments*/,
                  const Pose& /*pose*/, const Intrinsics& /*intr*/, int /*thickness*/) {
    // TODO(owner, D-06): flatten endpoints, one cv::projectPoints call, cv::line per segment.
}

}  // namespace calibar
