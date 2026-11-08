/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Projects D-07 board-frame geometry onto the image, so apps describe AR objects as 3D
           segments and never touch projectPoints themselves.
*/
#include "core/draw.h"

#include <opencv2/geometry.hpp>  // projectPoints moved here from calib3d in OpenCV 5
#include <opencv2/imgproc.hpp>

namespace calibar {

namespace {

// Point3f in -> Point2f out: projectPoints asserts the output precision matches the input (H-07).
std::vector<cv::Point2f> project(const std::vector<cv::Point3f>& pts, const Pose& pose,
                                 const Intrinsics& intr) {
    std::vector<cv::Point2f> px;
    cv::projectPoints(pts, pose.rvec, pose.tvec, intr.cameraMatrix, intr.distCoeffs, px);
    return px;
}

}  // namespace

void drawAxes(cv::Mat& img, const Pose& pose, const Intrinsics& intr, float length) {
    const std::vector<cv::Point3f> pts = {
        {0, 0, 0}, {length, 0, 0}, {0, length, 0}, {0, 0, length}};
    const std::vector<cv::Point2f> px = project(pts, pose, intr);
    const cv::Scalar colors[] = {{0, 0, 255}, {0, 255, 0}, {255, 0, 0}};  // X red, Y green, Z blue
    for (int i = 0; i < 3; ++i) {
        cv::arrowedLine(img, px[0], px[i + 1], colors[i], 3, cv::LINE_AA, 0, 0.15);
    }
}

void drawSegments(cv::Mat& img, const std::vector<Segment3D>& segments, const Pose& pose,
                  const Intrinsics& intr, int thickness) {
    if (segments.empty()) return;
    // Endpoints flattened as [a0, b0, a1, b1, ...]: one projectPoints call instead of one per
    // segment, and segment i's endpoints come back at 2i and 2i + 1.
    std::vector<cv::Point3f> pts;
    pts.reserve(segments.size() * 2);
    for (const Segment3D& s : segments) {
        pts.push_back(s.a);
        pts.push_back(s.b);
    }
    const std::vector<cv::Point2f> px = project(pts, pose, intr);
    for (size_t i = 0; i < segments.size(); ++i) {
        cv::line(img, px[2 * i], px[2 * i + 1], segments[i].color, thickness, cv::LINE_AA);
    }
}

}  // namespace calibar
