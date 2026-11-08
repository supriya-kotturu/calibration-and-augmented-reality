/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Persists intrinsics between calibrate (writer) and ar (reader). Key layout is the
           contract in intrinsics_io.h; resolution is stored because intrinsics only hold at it.
*/
#include "core/intrinsics_io.h"

#include <stdexcept>

namespace calibar {

void saveIntrinsics(const std::string& path, const Intrinsics& in) {
    // OpenCV throws on some bad paths and just fails to open on others; both become the
    // runtime_error the header promises, so callers handle one failure mode.
    cv::FileStorage fs;
    try {
        fs.open(path, cv::FileStorage::WRITE);
    } catch (const cv::Exception& e) {
        throw std::runtime_error("cannot write intrinsics to '" + path + "': " + e.what());
    }
    if (!fs.isOpened()) throw std::runtime_error("cannot write intrinsics to '" + path + "'");

    fs << "camera_matrix" << in.cameraMatrix;
    fs << "dist_coeffs" << in.distCoeffs;
    fs << "image_width" << in.imageSize.width;
    fs << "image_height" << in.imageSize.height;
    fs << "rms_px" << in.rmsPx;
    fs << "num_frames" << in.numFrames;
}

std::optional<Intrinsics> loadIntrinsics(const std::string& path) {
    // A missing or malformed file is an expected state (not calibrated yet), so it is nullopt,
    // never an exception.
    try {
        cv::FileStorage fs(path, cv::FileStorage::READ);
        if (!fs.isOpened()) return std::nullopt;

        // Check every key first: >> into a missing node silently leaves the default (e.g. 0), which
        // would load a half-written file as if it were valid.
        for (const char* key : {"camera_matrix", "dist_coeffs", "image_width", "image_height",
                                "rms_px", "num_frames"}) {
            if (fs[key].empty()) return std::nullopt;
        }

        Intrinsics out;
        int width = 0;
        int height = 0;
        fs["camera_matrix"] >> out.cameraMatrix;
        fs["dist_coeffs"] >> out.distCoeffs;
        fs["image_width"] >> width;
        fs["image_height"] >> height;
        fs["rms_px"] >> out.rmsPx;
        fs["num_frames"] >> out.numFrames;
        out.imageSize = cv::Size(width, height);
        return out;
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
}

}  // namespace calibar
