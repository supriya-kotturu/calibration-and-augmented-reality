/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Virtual line objects for the AR app (Task 6), in the D-07 board frame. Header-only
           so WS2 needs no CMake change; move to src/core/objects.cpp if that becomes preferable.
*/

#pragma once

#include <vector>

#include "core/draw.h"

namespace calibar {

// 2x2x2 cube sitting on the board, centred on the board's middle. Symmetric on purpose: for
// debugging the pose, not for the demo. Bottom red, top green, pillars blue.
inline std::vector<Segment3D> debugCube() {
    const cv::Scalar bottom(0, 0, 255), top(0, 255, 0), pillar(255, 0, 0);
    const float x0 = 2.5f, x1 = 4.5f, y0 = -3.0f, y1 = -1.0f, z1 = 2.0f;
    std::vector<Segment3D> s;
    const cv::Point3f b[4] = {{x0, y0, 0}, {x1, y0, 0}, {x1, y1, 0}, {x0, y1, 0}};
    for (int i = 0; i < 4; ++i) {
        const cv::Point3f& bn = b[(i + 1) % 4];
        const cv::Point3f t{b[i].x, b[i].y, z1}, tn{bn.x, bn.y, z1};
        s.push_back({b[i], bn, bottom});
        s.push_back({t, tn, top});
        s.push_back({b[i], t, pillar});
    }
    return s;
}

// A house hovering one square above the board. Asymmetric so a flipped or mirrored pose is
// obvious: the chimney sits on the +x/+y side, the door only on the -y face, the roof ridge
// runs along x. Base white, walls yellow, roof red, chimney magenta, door cyan.
inline std::vector<Segment3D> house() {
    const cv::Scalar wall(0, 220, 255), roof(0, 0, 255), chimney(255, 0, 255), door(255, 255, 0),
        base(255, 255, 255);
    const float x0 = 2.0f, x1 = 5.0f, y0 = -3.0f, y1 = -1.0f, zb = 1.0f, zw = 2.5f, zr = 3.8f;
    const float ym = (y0 + y1) / 2;
    std::vector<Segment3D> s;

    const cv::Point3f bot[4] = {{x0, y0, zb}, {x1, y0, zb}, {x1, y1, zb}, {x0, y1, zb}};
    const cv::Point3f top[4] = {{x0, y0, zw}, {x1, y0, zw}, {x1, y1, zw}, {x0, y1, zw}};
    for (int i = 0; i < 4; ++i) {
        s.push_back({bot[i], bot[(i + 1) % 4], base});
        s.push_back({top[i], top[(i + 1) % 4], wall});
        s.push_back({bot[i], top[i], wall});
    }

    const cv::Point3f ridgeL{x0, ym, zr}, ridgeR{x1, ym, zr};
    s.push_back({ridgeL, ridgeR, roof});
    s.push_back({top[0], ridgeL, roof});
    s.push_back({top[3], ridgeL, roof});
    s.push_back({top[1], ridgeR, roof});
    s.push_back({top[2], ridgeR, roof});

    // The chimney foot sits on the +y roof slope, not at wall-top height, or it would float.
    const float cx0 = 4.1f, cx1 = 4.6f, cy0 = -1.7f, cy1 = -1.2f, cz1 = 4.6f;
    const float slope = (zr - zw) / (y1 - ym);
    const auto roofZ = [&](float y) { return zw + slope * (y1 - y); };
    const cv::Point3f f[4] = {{cx0, cy0, roofZ(cy0)},
                              {cx1, cy0, roofZ(cy0)},
                              {cx1, cy1, roofZ(cy1)},
                              {cx0, cy1, roofZ(cy1)}};
    for (int i = 0; i < 4; ++i) {
        const cv::Point3f& fn = f[(i + 1) % 4];
        const cv::Point3f t0{f[i].x, f[i].y, cz1}, t1{fn.x, fn.y, cz1};
        s.push_back({f[i], fn, chimney});
        s.push_back({t0, t1, chimney});
        s.push_back({f[i], t0, chimney});
    }

    // Door on the y0 face, off-centre toward -x.
    const float dx0 = 2.4f, dx1 = 3.1f, dz1 = 2.2f;
    s.push_back({{dx0, y0, zb}, {dx0, y0, dz1}, door});
    s.push_back({{dx0, y0, dz1}, {dx1, y0, dz1}, door});
    s.push_back({{dx1, y0, dz1}, {dx1, y0, zb}, door});
    return s;
}

}  // namespace calibar
