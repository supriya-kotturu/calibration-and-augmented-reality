/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Round-trip and failure-mode tests for intrinsics YAML IO, plus a check that the
           committed fixture parses. Spec for the owner's saveIntrinsics/loadIntrinsics (D-06).
*/
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "core/intrinsics_io.h"

using namespace calibar;
namespace fs = std::filesystem;

namespace {

fs::path tempPath(const std::string& name) {
    return fs::temp_directory_path() / ("calibar_" + name);
}

Intrinsics sample() {
    Intrinsics in;
    in.cameraMatrix = cv::Mat(cv::Matx33d(1000.5, 0, 639.25, 0, 998.75, 360.5, 0, 0, 1)).clone();
    in.distCoeffs = cv::Mat(std::vector<double>{0.1, -0.25, 0.001, -0.002, 0.05});
    in.imageSize = {1280, 720};
    in.rmsPx = 0.4375;
    in.numFrames = 12;
    return in;
}

}  // namespace

TEST(IntrinsicsIo, RoundTripPreservesEverything) {
    const auto path = tempPath("roundtrip.yml");
    fs::remove(path);
    const Intrinsics in = sample();
    saveIntrinsics(path.string(), in);

    const auto out = loadIntrinsics(path.string());
    ASSERT_TRUE(out.has_value()) << "file written by saveIntrinsics did not load";
    EXPECT_EQ(out->cameraMatrix.type(), CV_64F) << "cameraMatrix must be CV_64F";
    ASSERT_EQ(out->cameraMatrix.size(), cv::Size(3, 3));
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            EXPECT_NEAR(out->cameraMatrix.at<double>(r, c), in.cameraMatrix.at<double>(r, c), 1e-9)
                << "K(" << r << "," << c << ")";
        }
    }
    EXPECT_EQ(out->distCoeffs.total(), in.distCoeffs.total());
    for (size_t i = 0; i < in.distCoeffs.total(); ++i) {
        EXPECT_NEAR(out->distCoeffs.at<double>(static_cast<int>(i)),
                    in.distCoeffs.at<double>(static_cast<int>(i)), 1e-9)
            << "dist[" << i << "]";
    }
    EXPECT_EQ(out->imageSize, in.imageSize);
    EXPECT_NEAR(out->rmsPx, in.rmsPx, 1e-9);
    EXPECT_EQ(out->numFrames, in.numFrames);
    fs::remove(path);
}

TEST(IntrinsicsIo, MissingFileGivesNullopt) {
    EXPECT_FALSE(loadIntrinsics(tempPath("does_not_exist.yml").string()).has_value());
}

TEST(IntrinsicsIo, FileMissingKeysGivesNullopt) {
    const auto path = tempPath("partial.yml");
    {
        std::ofstream f(path);
        f << "%YAML:1.0\n---\nimage_width: 1280\nimage_height: 720\n";
    }
    EXPECT_FALSE(loadIntrinsics(path.string()).has_value())
        << "a file without camera_matrix/dist_coeffs must not load";
    fs::remove(path);
}

TEST(IntrinsicsIo, SaveToUnwritablePathThrows) {
    const auto path = tempPath("no_such_dir") / "x" / "k.yml";
    EXPECT_THROW(saveIntrinsics(path.string(), sample()), std::runtime_error);
}

// Runs from the repo root (see tests/CMakeLists.txt). AR work develops against this file.
TEST(IntrinsicsIo, FixtureLoadsWithExpectedShape) {
    const auto k = loadIntrinsics("data/calib/fixture.yml");
    ASSERT_TRUE(k.has_value()) << "data/calib/fixture.yml missing or wrong layout";
    ASSERT_EQ(k->cameraMatrix.size(), cv::Size(3, 3));
    const double w = k->imageSize.width, h = k->imageSize.height;
    EXPECT_NEAR(k->cameraMatrix.at<double>(0, 0), 0.9 * w, 1e-6) << "fx = 0.9 * width";
    EXPECT_NEAR(k->cameraMatrix.at<double>(1, 1), 0.9 * w, 1e-6) << "fy = fx";
    EXPECT_NEAR(k->cameraMatrix.at<double>(0, 2), w / 2, 1e-6) << "cx at centre";
    EXPECT_NEAR(k->cameraMatrix.at<double>(1, 2), h / 2, 1e-6) << "cy at centre";
    EXPECT_NEAR(cv::norm(k->distCoeffs), 0.0, 1e-12) << "fixture has zero distortion";
}
