/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Intrinsics YAML IO. Owner implements by hand (D-06); stub until then.
*/
#include "core/intrinsics_io.h"

namespace calibar {

void saveIntrinsics(const std::string& /*path*/, const Intrinsics& /*in*/) {
    // TODO(owner, D-06): write the keys documented in intrinsics_io.h with cv::FileStorage
    // (FileStorage::WRITE); throw std::runtime_error if !fs.isOpened().
}

std::optional<Intrinsics> loadIntrinsics(const std::string& /*path*/) {
    // TODO(owner, D-06): read the keys with cv::FileStorage (FileStorage::READ); return
    // std::nullopt if the file is missing or a key is absent (FileNode::empty()).
    return std::nullopt;
}

}  // namespace calibar
