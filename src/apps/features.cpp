/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Task 7: live robust-feature display (Harris / ORB / SIFT) on any video source, with
           trackbars to experiment with detector settings and an optional ORB match against a
           reference frame to illustrate how features could anchor AR without a fiducial.
*/
#include <opencv2/core.hpp>
#include <opencv2/features.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "core/app_args.h"
#include "core/video_source.h"

namespace {

using Clock = std::chrono::steady_clock;

// The sample video is 1080x1920 portrait; detectors at full size are slow and the window would not
// fit a screen. Everything (headless numbers included) runs at this long-side size so the two
// agree.
constexpr int kMaxSide = 960;

constexpr int kKsizeTable[] = {1, 3, 5, 7};  // cornerHarris needs odd aperture sizes
constexpr const char* kWin = "features";
constexpr const char* kShotDir = "data/features";

enum class Mode { Harris, Orb, Sift };

const char* modeName(Mode m) {
    switch (m) {
        case Mode::Harris:
            return "Harris";
        case Mode::Orb:
            return "ORB";
        case Mode::Sift:
            return "SIFT";
    }
    return "?";
}

// Detector parameters; in live mode readTrackbars refreshes them from the sliders each frame.
struct Params {
    int blockSize = 2;       // Harris neighbourhood; min 2
    int ksizeIdx = 1;        // index into kKsizeTable -> 3
    int kx1000 = 40;         // Harris k * 1000 (0.04, the textbook value)
    int threshold = 10;      // per-mille of the max Harris response (10 = 1%)
    int nfeatures = 500;     // ORB
    int fastThreshold = 20;  // ORB FAST threshold (OpenCV default)
};

struct Result {
    int count = 0;
    cv::Mat vis;
};

cv::Mat downscale(const cv::Mat& frame) {
    const int longSide = std::max(frame.cols, frame.rows);
    if (longSide <= kMaxSide) return frame;
    cv::Mat out;
    const double s = double(kMaxSide) / longSide;
    cv::resize(frame, out, cv::Size(), s, s, cv::INTER_AREA);
    return out;
}

Result runHarris(const cv::Mat& bgr, const cv::Mat& gray, const Params& p) {
    const int block = std::max(2, p.blockSize);
    const int ksize = kKsizeTable[std::clamp(p.ksizeIdx, 0, 3)];
    cv::Mat resp;
    cv::cornerHarris(gray, resp, block, ksize, p.kx1000 / 1000.0);
    // Threshold relative to the strongest corner: min-max normalising would let the large
    // negative (edge) responses shift zero and make the threshold meaningless across frames.
    double maxResp = 0;
    cv::minMaxLoc(resp, nullptr, &maxResp);
    const cv::Mat& norm = resp;
    const float cut = float(maxResp * p.threshold / 1000.0);

    // Non-max suppression: otherwise one corner counts as dozens of adjacent pixels.
    cv::Mat dil;
    cv::dilate(norm, dil, cv::Mat());
    Result r;
    r.vis = bgr.clone();
    for (int y = 0; y < norm.rows; ++y) {
        const float* n = norm.ptr<float>(y);
        const float* d = dil.ptr<float>(y);
        for (int x = 0; x < norm.cols; ++x) {
            if (n[x] > cut && n[x] == d[x]) {
                ++r.count;
                cv::circle(r.vis, {x, y}, 4, {0, 0, 255}, 1, cv::LINE_AA);
            }
        }
    }
    return r;
}

Result runKeypoints(const cv::Ptr<cv::Feature2D>& det, const cv::Mat& bgr, const cv::Mat& gray) {
    std::vector<cv::KeyPoint> kps;
    det->detect(gray, kps);
    Result r;
    r.count = int(kps.size());
    cv::drawKeypoints(bgr, kps, r.vis, cv::Scalar::all(-1),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    return r;
}

// ORB is rebuilt when its trackbars change because nfeatures / fastThreshold are fixed at
// construction.
struct Detectors {
    cv::Ptr<cv::ORB> orb;
    cv::Ptr<cv::SIFT> sift;
    int orbN = -1, orbFast = -1;

    Detectors() {
        try {
            sift = cv::SIFT::create();
        } catch (const cv::Exception& e) {
            std::cerr << "SIFT unavailable (" << e.what() << "); mode 's' disabled\n";
        }
    }
    cv::Ptr<cv::ORB>& orbFor(const Params& p) {
        const int n = std::max(1, p.nfeatures), f = std::max(1, p.fastThreshold);
        if (!orb || n != orbN || f != orbFast) {
            orb = cv::ORB::create(n, 1.2f, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31, f);
            orbN = n;
            orbFast = f;
        }
        return orb;
    }
};

// Ratio-test match of the current frame's ORB features against the stored reference. Shows
// that features alone give frame-to-frame correspondences, which is what AR on an arbitrary
// target (homography / solvePnP from matches) would be built on.
struct Reference {
    cv::Mat bgr, gray, desc;
    std::vector<cv::KeyPoint> kps;
    bool valid() const { return !desc.empty(); }
};

Result runMatch(const cv::Ptr<cv::ORB>& orb, const Reference& ref, const cv::Mat& bgr,
                const cv::Mat& gray) {
    std::vector<cv::KeyPoint> kps;
    cv::Mat desc;
    orb->detectAndCompute(gray, cv::noArray(), kps, desc);
    std::vector<cv::DMatch> good;
    if (!desc.empty()) {
        std::vector<std::vector<cv::DMatch>> knn;
        cv::BFMatcher(cv::NORM_HAMMING).knnMatch(desc, ref.desc, knn, 2);
        for (const auto& m : knn)
            if (m.size() == 2 && m[0].distance < 0.75f * m[1].distance) good.push_back(m[0]);
    }
    Result r;
    r.count = int(good.size());
    cv::drawMatches(bgr, kps, ref.bgr, ref.kps, good, r.vis, cv::Scalar::all(-1),
                    cv::Scalar::all(-1), {}, cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    return r;
}

void putHud(cv::Mat& img, const std::vector<std::string>& lines) {
    int y = 22;
    for (const auto& s : lines) {
        cv::putText(img, s, {9, y + 1}, cv::FONT_HERSHEY_SIMPLEX, 0.55, {0, 0, 0}, 3, cv::LINE_AA);
        cv::putText(img, s, {8, y}, cv::FONT_HERSHEY_SIMPLEX, 0.55, {255, 255, 255}, 1,
                    cv::LINE_AA);
        y += 22;
    }
}

std::string paramText(Mode m, const Params& p) {
    char buf[128];
    if (m == Mode::Harris)
        std::snprintf(buf, sizeof buf, "block=%d ksize=%d k=%.3f thr=%d/1000", std::max(2, p.blockSize),
                      kKsizeTable[std::clamp(p.ksizeIdx, 0, 3)], p.kx1000 / 1000.0, p.threshold);
    else if (m == Mode::Orb)
        std::snprintf(buf, sizeof buf, "nfeatures=%d fastThr=%d", p.nfeatures, p.fastThreshold);
    else
        std::snprintf(buf, sizeof buf, "defaults (no trackbars)");
    return buf;
}

void rebuildTrackbars(Mode m, Params& p) {
    // highgui has no remove-trackbar call, so recreate the window per mode. The Win32 backend
    // throws on destroying a window that doesn't exist yet, so skip that on the first call.
    static bool windowExists = false;
    if (windowExists) cv::destroyWindow(kWin);
    cv::namedWindow(kWin, cv::WINDOW_AUTOSIZE);
    windowExists = true;
    // OpenCV 5 deprecates the value-pointer form of createTrackbar, so create each slider with
    // nullptr, seed it from Params, and poll it in readTrackbars.
    auto bar = [](const char* name, int value, int max) {
        cv::createTrackbar(name, kWin, nullptr, max);
        cv::setTrackbarPos(name, kWin, value);
    };
    if (m == Mode::Harris) {
        bar("blockSize", p.blockSize, 15);
        bar("ksize 1/3/5/7", p.ksizeIdx, 3);
        bar("k x1000", p.kx1000, 100);
        bar("threshold (permille of max)", p.threshold, 200);
    } else if (m == Mode::Orb) {
        bar("nfeatures", p.nfeatures, 5000);
        bar("fastThreshold", p.fastThreshold, 100);
    }
}

void readTrackbars(Mode m, Params& p) {
    if (m == Mode::Harris) {
        p.blockSize = cv::getTrackbarPos("blockSize", kWin);
        p.ksizeIdx = cv::getTrackbarPos("ksize 1/3/5/7", kWin);
        p.kx1000 = cv::getTrackbarPos("k x1000", kWin);
        p.threshold = cv::getTrackbarPos("threshold (permille of max)", kWin);
    } else if (m == Mode::Orb) {
        p.nfeatures = cv::getTrackbarPos("nfeatures", kWin);
        p.fastThreshold = cv::getTrackbarPos("fastThreshold", kWin);
    }
}

int runHeadless(cv::VideoCapture& cap, int maxFrames) {
    const int limit = maxFrames > 0 ? maxFrames : 100;
    Params p;
    Detectors det;
    constexpr Mode kModes[] = {Mode::Harris, Mode::Orb, Mode::Sift};
    double totalMs[3] = {}, totalCount[3] = {};
    int frames = 0;
    cv::Mat frame;
    while (frames < limit && cap.read(frame)) {
        const cv::Mat bgr = downscale(frame);
        cv::Mat gray;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
        for (int i = 0; i < 3; ++i) {
            if (kModes[i] == Mode::Sift && !det.sift) continue;
            const auto t0 = Clock::now();
            Result r = kModes[i] == Mode::Harris ? runHarris(bgr, gray, p)
                       : kModes[i] == Mode::Orb  ? runKeypoints(det.orbFor(p), bgr, gray)
                                                 : runKeypoints(det.sift, bgr, gray);
            totalMs[i] += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
            totalCount[i] += r.count;
        }
        ++frames;
    }
    if (frames == 0) {
        std::cerr << "features: no frames read\n";
        return 1;
    }
    std::printf("features headless: %d frames at <=%dpx long side (detector time only)\n", frames,
                kMaxSide);
    for (int i = 0; i < 3; ++i) {
        if (kModes[i] == Mode::Sift && !det.sift) {
            std::printf("  %-6s unavailable\n", modeName(kModes[i]));
            continue;
        }
        std::printf("  %-6s avg features %8.1f  avg %7.2f ms  fps %6.1f  (%s)\n",
                    modeName(kModes[i]), totalCount[i] / frames, totalMs[i] / frames,
                    1000.0 * frames / totalMs[i], paramText(kModes[i], p).c_str());
    }
    return 0;
}

void printHelp() {
    std::cout << "keys: h Harris | o ORB | s SIFT | r capture reference frame | m toggle ORB match "
                 "vs reference | p screenshot | q/ESC quit\n";
}

}  // namespace

int main(int argc, char** argv) {
    const calibar::AppArgs args = calibar::parseAppArgs(argc, argv);
    if (!args.ok) {
        std::cerr << args.error << "\n" << calibar::appUsage("features") << "\n";
        return 2;
    }
    cv::VideoCapture cap = calibar::openSource(args.source);
    if (!cap.isOpened()) {
        std::cerr << "features: cannot open source '" << args.source << "'\n";
        return 1;
    }
    if (args.headless) return runHeadless(cap, args.maxFrames);

    printHelp();
    Params p;
    Detectors det;
    Mode mode = Mode::Harris;
    bool matching = false;
    Reference ref;
    rebuildTrackbars(mode, p);

    auto setMode = [&](Mode m) {
        if (m == Mode::Sift && !det.sift) {
            std::cout << "SIFT unavailable in this OpenCV build\n";
            return;
        }
        if (m != mode) {
            mode = m;
            rebuildTrackbars(mode, p);
        }
        if (mode != Mode::Orb) matching = false;
    };

    double fps = 0;
    int shot = 0;
    int framesRead = 0;
    cv::Mat frame, lastShown;
    auto last = Clock::now();
    while (cap.read(frame)) {
        if (args.maxFrames > 0 && framesRead++ >= args.maxFrames) break;
        readTrackbars(mode, p);
        const cv::Mat bgr = downscale(frame);
        cv::Mat gray;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

        Result r;
        if (matching && ref.valid())
            r = runMatch(det.orbFor(p), ref, bgr, gray);
        else if (mode == Mode::Harris)
            r = runHarris(bgr, gray, p);
        else if (mode == Mode::Orb)
            r = runKeypoints(det.orbFor(p), bgr, gray);
        else
            r = runKeypoints(det.sift, bgr, gray);

        const auto now = Clock::now();
        const double inst = 1.0 / std::chrono::duration<double>(now - last).count();
        last = now;
        fps = fps == 0 ? inst : 0.9 * fps + 0.1 * inst;

        putHud(r.vis, {std::string("mode: ") + (matching ? "ORB match" : modeName(mode)),
                       std::string(matching ? "good matches: " : "features: ") +
                           std::to_string(r.count),
                       "fps: " + std::to_string(int(fps + 0.5)), paramText(mode, p)});
        cv::imshow(kWin, r.vis);
        lastShown = r.vis;

        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == 'h') setMode(Mode::Harris);
        if (key == 'o') setMode(Mode::Orb);
        if (key == 's') setMode(Mode::Sift);
        if (key == 'r') {
            ref.bgr = bgr.clone();
            ref.gray = gray.clone();
            det.orbFor(p)->detectAndCompute(ref.gray, cv::noArray(), ref.kps, ref.desc);
            std::cout << "reference captured: " << ref.kps.size() << " ORB keypoints\n";
        }
        if (key == 'm') {
            if (!ref.valid()) {
                std::cout << "press r to capture a reference frame first\n";
            } else {
                matching = !matching;
                if (matching && mode != Mode::Orb) setMode(Mode::Orb);
            }
        }
        if (key == 'p' && !lastShown.empty()) {
            std::filesystem::create_directories(kShotDir);
            // The counter restarts every run; skip names already on disk so earlier shots survive.
            char name[64];
            do {
                std::snprintf(name, sizeof name, "%s/shot_%03d.png", kShotDir, shot++);
            } while (std::filesystem::exists(name));
            cv::imwrite(name, lastShown);
            std::cout << "saved " << name << "\n";
        }
    }
    cv::destroyAllWindows();
    return 0;
}
