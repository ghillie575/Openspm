cmake_minimum_required(VERSION 3.10.0)

project(openspm VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
include(CTest)
find_package(PkgConfig REQUIRED)

# ─────────────────────────────────────────────────────────────
# ZSTD (system)

pkg_check_modules(ZSTD REQUIRED libzstd)

add_library(zstd::libzstd INTERFACE IMPORTED)
target_include_directories(zstd::libzstd INTERFACE ${ZSTD_INCLUDE_DIRS})
target_link_libraries(zstd::libzstd INTERFACE ${ZSTD_LIBRARIES})

# ─────────────────────────────────────────────────────────────
# libarchive (system)

pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# ─────────────────────────────────────────────────────────────
# ncurses (system)

find_package(Curses REQUIRED)

# ─────────────────────────────────────────────────────────────
# Fetch dependencies (NO indicators anymore)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)

FetchContent_Declare(
  cpphttplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)

FetchContent_MakeAvailable(yaml-cpp cpphttplib)

# ─────────────────────────────────────────────────────────────
# Sources

file(GLOB SOURCES
  "src/*.cpp"
  "src/*.h"
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(openspm_lib ${SOURCES})
add_executable(openspm main.cpp)

# ─────────────────────────────────────────────────────────────
# Version info

set(OPENSPM_VERSION_STRING ${PROJECT_VERSION})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPENSPM_VERSION_STRING "${PROJECT_VERSION}-dev")
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%MZ" UTC)

target_compile_definitions(openspm_lib PRIVATE
  OPENSPM_VERSION="${OPENSPM_VERSION_STRING}"
  OPENSPM_BUILD_DATE="${BUILD_DATE}"
)

# ─────────────────────────────────────────────────────────────
# Include directories

target_include_directories(openspm_lib
  PUBLIC
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${CURSES_INCLUDE_DIRS}
)

# ─────────────────────────────────────────────────────────────
# Link libraries

target_link_libraries(openspm_lib
  PUBLIC
    yaml-cpp::yaml-cpp
    httplib::httplib
    zstd::libzstd
    ${LIBARCHIVE_LIBRARIES}
    ${CURSES_LIBRARIES}
)

target_link_libraries(openspm
  PUBLIC
    openspm_lib
)

# ─────────────────────────────────────────────────────────────
# Tests

enable_testing()
add_subdirectory(tests)

# ─────────────────────────────────────────────────────────────
# Packaging

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
