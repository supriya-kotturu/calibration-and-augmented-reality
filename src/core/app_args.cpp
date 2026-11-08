/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Implements the shared CLI parser.
*/
#include "core/app_args.h"

#include <exception>
#include <stdexcept>

namespace calibar {

namespace {

// Returns the partly parsed args marked invalid, so callers print usage and exit 2.
AppArgs fail(AppArgs a, std::string msg) {
    a.ok = false;
    a.error = std::move(msg);
    return a;
}

}  // namespace

AppArgs parseAppArgs(int argc, const char* const* argv) {
    AppArgs a;
    bool haveSource = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--headless") {
            a.headless = true;
        } else if (arg == "--intrinsics") {
            if (i + 1 >= argc) return fail(a, "--intrinsics needs a path");
            a.intrinsicsPath = argv[++i];
        } else if (arg == "--max-frames") {
            if (i + 1 >= argc) return fail(a, "--max-frames needs a number");
            const std::string v = argv[++i];
            try {
                std::size_t used = 0;
                a.maxFrames = std::stoi(v, &used);
                if (used != v.size() || a.maxFrames < 0) throw std::invalid_argument(v);
            } catch (const std::exception&) {
                return fail(a, "--max-frames needs a non-negative integer, got '" + v + "'");
            }
        } else if (arg.rfind("--", 0) == 0) {
            return fail(a, "unknown option " + arg);
        } else if (!haveSource) {
            a.source = arg;
            haveSource = true;
        } else {
            return fail(a, "unexpected extra argument " + arg);
        }
    }
    return a;
}

std::string appUsage(const std::string& prog) {
    return "usage: " + prog + " [source] [--intrinsics path] [--headless] [--max-frames N]";
}

}  // namespace calibar
