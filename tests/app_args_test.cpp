/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Tests the shared CLI parser every app relies on for headless verification.
*/
#include <gtest/gtest.h>

#include <vector>

#include "core/app_args.h"

using namespace calibar;

namespace {
AppArgs parse(std::vector<const char*> argv) {
    argv.insert(argv.begin(), "app");
    return parseAppArgs(static_cast<int>(argv.size()), argv.data());
}
}  // namespace

TEST(AppArgs, Defaults) {
    const auto a = parse({});
    EXPECT_TRUE(a.ok);
    EXPECT_EQ(a.source, "0");
    EXPECT_TRUE(a.intrinsicsPath.empty());
    EXPECT_FALSE(a.headless);
    EXPECT_EQ(a.maxFrames, 0);
}

TEST(AppArgs, AllFlags) {
    const auto a = parse({"x.mp4", "--intrinsics", "k.yml", "--headless", "--max-frames", "30"});
    EXPECT_TRUE(a.ok) << a.error;
    EXPECT_EQ(a.source, "x.mp4");
    EXPECT_EQ(a.intrinsicsPath, "k.yml");
    EXPECT_TRUE(a.headless);
    EXPECT_EQ(a.maxFrames, 30);
}

TEST(AppArgs, FlagsBeforeSource) {
    const auto a = parse({"--headless", "clip.mp4"});
    EXPECT_TRUE(a.ok) << a.error;
    EXPECT_EQ(a.source, "clip.mp4");
    EXPECT_TRUE(a.headless);
}

TEST(AppArgs, RejectsBadInput) {
    EXPECT_FALSE(parse({"--bogus"}).ok);
    EXPECT_FALSE(parse({"a", "b"}).ok) << "two sources";
    EXPECT_FALSE(parse({"--intrinsics"}).ok) << "missing value";
    EXPECT_FALSE(parse({"--max-frames"}).ok) << "missing value";
    EXPECT_FALSE(parse({"--max-frames", "abc"}).ok);
    EXPECT_FALSE(parse({"--max-frames", "-3"}).ok);
    EXPECT_FALSE(parse({"--max-frames", "5x"}).ok);
    EXPECT_FALSE(parse({"--max-frames", "-3"}).error.empty()) << "error must say why";
}
