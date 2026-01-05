cmake_minimum_required(VERSION 3.10.0)
project(openspm VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)
find_package(PkgConfig REQUIRED)
# ZSTD
pkg_check_modules(ZSTD REQUIRED libzstd)
add_library(zstd::libzstd INTERFACE IMPORTED)
target_include_directories(zstd::libzstd INTERFACE ${ZSTD_INCLUDE_DIRS})
target_link_libraries(zstd::libzstd INTERFACE ${ZSTD_LIBRARIES})
# YAML-CPP and CPPHTTPLIB
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)
FetchContent_Declare(
  cpphttplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)

FetchContent_MakeAvailable(cpphttplib yaml-cpp)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(openspm main.cpp ${SOURCES})
set(OPENSPM_VERSION_STRING ${PROJECT_VERSION})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPENSPM_VERSION_STRING "${PROJECT_VERSION}-dev")
endif()
target_compile_definitions(openspm PRIVATE OPENSPM_VERSION="${OPENSPM_VERSION_STRING}")
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%MZ" UTC)
target_compile_definitions(openspm PRIVATE OPENSPM_BUILD_DATE="${BUILD_DATE}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

target_include_directories(openspm PRIVATE ${LIBARCHIVE_INCLUDE_DIRS})
target_link_libraries(openspm PUBLIC yaml-cpp::yaml-cpp httplib::httplib ${LIBARCHIVE_LIBRARIES})

# Doxygen configuration for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    # Copy Doxyfile to build directory
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} COPYONLY)
    
    # Create custom target for Doxygen
    add_custom_target(doxygen
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )
    
    message(STATUS "Doxygen found. Use 'make doxygen' or 'cmake --build . --target doxygen' to generate documentation.")
  else()
    message(WARNING "Doxygen not found. Documentation will not be generated.")
  endif()
endif()

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
