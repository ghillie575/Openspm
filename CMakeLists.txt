cmake_minimum_required(VERSION 3.10.0)
project(openspm VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)
FetchContent_Declare(
  cpphttplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)

# Make available (libzip is now expected from the system)
FetchContent_MakeAvailable(cpphttplib yaml-cpp)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(openspm main.cpp ${SOURCES})
target_compile_definitions(openspm PRIVATE OPENSPM_VERSION="${PROJECT_VERSION}")
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%MZ" UTC)
target_compile_definitions(openspm PRIVATE OPENSPM_BUILD_DATE="${BUILD_DATE}")

# Find system libzip (via pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

target_include_directories(openspm PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(openspm PUBLIC yaml-cpp::yaml-cpp ${LIBZIP_LIBRARIES})

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
