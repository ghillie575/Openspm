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
# Fetch dependencies

FetchContent_Declare(
  indicators
  GIT_REPOSITORY https://github.com/p-ranav/indicators.git
  GIT_TAG master
)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)

FetchContent_Declare(
  cpphttplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master
)
FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(indicators yaml-cpp cpphttplib doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)
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
  set(OPENSPM_VERSION_STRING "${PROJECT_VERSION}-dev-${GIT_COMMIT_HASH}")
endif()

target_compile_definitions(openspm_lib PRIVATE
  OPENSPM_VERSION="${OPENSPM_VERSION_STRING}"
)

# ─────────────────────────────────────────────────────────────
# Include directories

target_include_directories(openspm_lib
  PUBLIC
  ${LIBARCHIVE_INCLUDE_DIRS}
)

# ─────────────────────────────────────────────────────────────
# Link libraries

target_link_libraries(openspm_lib
  PUBLIC
  indicators::indicators
  yaml-cpp::yaml-cpp
  httplib::httplib
  zstd::libzstd
  ${LIBARCHIVE_LIBRARIES}
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
# Doxygen documentation (Release builds only)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    # Set Doxygen input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Configure Doxygen file (if needed for variable substitution)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    # Add custom target to generate documentation
    add_custom_target(doc_doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )
    
    message(STATUS "Doxygen documentation will be generated in Release build")
  else()
    message(WARNING "Doxygen not found. Documentation will not be generated.")
  endif()
endif()

# ─────────────────────────────────────────────────────────────
# Packaging

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
