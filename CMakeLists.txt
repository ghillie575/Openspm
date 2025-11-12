cmake_minimum_required(VERSION 3.10.0)
project(openspm VERSION 0.1.0 LANGUAGES C CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# === YAML-CPP ===
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

# === CPP-HTTPLIB === (header-only HTTP client/server library)
FetchContent_Declare(
  cpp-httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)
FetchContent_MakeAvailable(cpp-httplib)

# === Collect source files ===
file(GLOB SOURCES "src/*.cpp" "src/*.h")
include_directories(${CMAKE_SOURCE_DIR}/include)

# === Create a common library ===
find_package(OpenSSL REQUIRED)
add_library(openspm-lib ${SOURCES})
target_link_libraries(openspm-lib PUBLIC yaml-cpp::yaml-cpp)
target_include_directories(openspm-lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(openspm-lib PUBLIC httplib::httplib)
target_link_libraries(openspm-lib PUBLIC OpenSSL::SSL OpenSSL::Crypto)

# === Executables ===
add_executable(openspm main.cpp)
add_executable(openspm-config openspm-config.cpp)

target_link_libraries(openspm PUBLIC openspm-lib)
target_link_libraries(openspm-config PUBLIC openspm-lib)

# === Testing ===
include(CTest)
enable_testing()

# === Packaging ===
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
