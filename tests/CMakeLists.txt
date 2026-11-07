include(FetchContent)
FetchContent_Declare(googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
# Match the MSVC runtime of the rest of the build (otherwise LNK2038 mismatches).
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(calibar_tests
	app_args_test.cpp
	intrinsics_io_test.cpp
	object_points_test.cpp
	pose_test.cpp
	synthetic_board.cpp
	video_source_test.cpp
)
target_link_libraries(calibar_tests PRIVATE calibar_core GTest::gtest_main)
add_dependencies(calibar_tests copy_opencv_dlls)

# Repo root as cwd so tests can open data/ files with relative paths.
gtest_discover_tests(calibar_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
