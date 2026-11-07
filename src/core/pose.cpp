/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Pose estimation from a ChArUco detection. Owner implements by hand (D-06); stub
           until then.
*/
#include "core/pose.h"

namespace calibar {

std::optional<Pose> estimatePose(const CharucoDetection& /*det*/, const Intrinsics& /*intr*/,
                                 int /*minCorners*/) {
    // TODO(owner, D-06): cv::solvePnP(objectPointsFor(det.ids), det.corners, K, dist, rvec, tvec,
    // false, SOLVEPNP_ITERATIVE); nullopt if too few corners or solvePnP fails.
    return std::nullopt;
}

}  // namespace calibar
