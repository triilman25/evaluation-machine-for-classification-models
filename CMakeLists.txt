cmake_minimum_required(VERSION 3.18)
project(model_eval VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 20)
set(FOR_WIN32 ON) #ubah ketika butuh untuk dijadikan aplikasi windows

if (FOR_WIN32)
    add_executable(model_eval WIN32 main.cpp)
else ()
    add_executable(model_eval main.cpp)
endif ()

set(RAYLIB_VERSION 5.5)
include(FetchContent)
FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(raylib)

# LibTorch Config
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_PREFIX_PATH "C:\\libtorch")
else()
    set(CMAKE_PREFIX_PATH "C:\\libtorch-2_5")
endif ()

find_package(Torch REQUIRED)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TORCH_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif (MSVC)

## OpenCV config
set(OpenCV_DIR "C:\\opencv\\build\\x64\\vc16\\lib")
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_subdirectory(lib)

target_link_libraries(${PROJECT_NAME} raylib element ${OpenCV_LIBS} libtransform ${TORCH_LIBRARIES}) #-mwindows
