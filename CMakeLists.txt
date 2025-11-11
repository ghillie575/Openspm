cmake_minimum_required(VERSION 3.10.0)
project(openspm VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master# Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)
file(GLOB SOURCES "src/*.cpp" "src/*.h")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(openspm-config openspm-config.cpp src/config.cpp)
add_executable(openspm main.cpp ${SOURCES})
target_link_libraries(openspm PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(openspm-config PUBLIC yaml-cpp::yaml-cpp)
include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
