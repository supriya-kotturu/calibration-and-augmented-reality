cmake_minimum_required(VERSION 3.20)
project(calibar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The prebuilt OpenCV at C:/opencv is MSVC-only; use the "msvc" preset so MinGW is never picked.
if((NOT OpenCV_DIR OR OpenCV_DIR MATCHES "-NOTFOUND$")
		AND EXISTS "C:/opencv/build/OpenCVConfig.cmake")
	set(OpenCV_DIR "C:/opencv/build" CACHE PATH "Directory containing OpenCVConfig.cmake" FORCE)
endif()
find_package(OpenCV REQUIRED)

# Executables land in out/ for every config so data paths stay relative to the repo root.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out")
foreach(config ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER "${config}" config_upper)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${PROJECT_SOURCE_DIR}/out")
endforeach()

if(MSVC)
	add_compile_options(/W4 /utf-8)
else()
	add_compile_options(-Wall -Wextra)
endif()

if(WIN32)
	file(GLOB OPENCV_RUNTIME_DLLS "${OpenCV_DIR}/x64/*/bin/*.dll" "${OpenCV_DIR}/bin/*.dll")
endif()

# Executables find the OpenCV DLLs by having them copied next to the exe. One shared target
# does the copy: per-exe copies race each other under parallel MSBuild.
add_custom_target(copy_opencv_dlls)
if(WIN32 AND OPENCV_RUNTIME_DLLS)
	add_custom_command(TARGET copy_opencv_dlls POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_RUNTIME_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
		COMMAND_EXPAND_LISTS
	)
endif()

function(add_calibar_executable target)
	add_executable(${target} ${ARGN})
	target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/include")
	target_link_libraries(${target} PRIVATE ${OpenCV_LIBS})
	set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
	add_dependencies(${target} copy_opencv_dlls)
endfunction()

# Everything shared between apps and tests lives in the static lib so the contracts are
# compiled once and tested the same way they are used.
add_library(calibar_core STATIC
	src/core/app_args.cpp
	src/core/charuco.cpp
	src/core/draw.cpp
	src/core/intrinsics_io.cpp
	src/core/pose.cpp
	src/core/video_source.cpp
)
target_include_directories(calibar_core PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(calibar_core PUBLIC ${OpenCV_LIBS})

add_calibar_executable(make_board src/apps/make_board.cpp)
add_calibar_executable(calibrate src/apps/calibrate.cpp)
add_calibar_executable(ar src/apps/ar.cpp)
add_calibar_executable(features src/apps/features.cpp)
foreach(app calibrate ar features)
	target_link_libraries(${app} PRIVATE calibar_core)
endforeach()

enable_testing()
add_subdirectory(tests)
