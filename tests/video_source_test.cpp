/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Tests the source-string parser and file path of openSource; never touches a camera.
*/
#include <gtest/gtest.h>

#include <filesystem>

#include "core/video_source.h"

using namespace calibar;

TEST(IsDeviceIndex, DigitsAreDevices) {
    EXPECT_TRUE(isDeviceIndex("0"));
    EXPECT_TRUE(isDeviceIndex("12"));
}

TEST(IsDeviceIndex, EverythingElseIsFileOrUrl) {
    EXPECT_FALSE(isDeviceIndex(""));
    EXPECT_FALSE(isDeviceIndex("x.mp4"));
    EXPECT_FALSE(isDeviceIndex("0.mp4"));
    EXPECT_FALSE(isDeviceIndex("-1"));
    EXPECT_FALSE(isDeviceIndex("http://192.168.1.5:8080/video"));
}

TEST(OpenSource, MissingFileIsNotOpened) {
    EXPECT_FALSE(openSource("definitely_not_here.mp4").isOpened());
}

TEST(OpenSource, SampleVideoOpensAndYieldsFrames) {
    const std::string path = "data/samples/board.mp4";
    // An un-pulled LFS pointer is a ~130 byte text file; say so instead of a vague failure.
    if (!std::filesystem::exists(path) || std::filesystem::file_size(path) < 1024) {
        GTEST_SKIP() << path << " missing or an LFS pointer (run `git lfs pull`)";
    }
    auto cap = openSource(path);
    ASSERT_TRUE(cap.isOpened());
    cv::Mat frame;
    ASSERT_TRUE(cap.read(frame));
    EXPECT_FALSE(frame.empty());
    EXPECT_EQ(frame.cols, static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)));
}
