cmake_minimum_required(VERSION 3.10.0)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project(openspm VERSION 0.1.1 LANGUAGES C CXX)

include(FetchContent)
include(CTest)

set(OPENSPM_BUILD_DOCS_DEFAULT ON)
if(WIN32)
  set(OPENSPM_BUILD_DOCS_DEFAULT OFF)
endif()
option(OPENSPM_BUILD_DOCS "Generate Doxygen documentation" ${OPENSPM_BUILD_DOCS_DEFAULT})

# Platform-specific package management

if(WIN32)
    find_package(PkgConfig QUIET)
    
    find_package(zstd CONFIG QUIET)
    find_package(LibArchive QUIET)
    
    if(NOT zstd_FOUND)
        FetchContent_Declare(
            zstd
            GIT_REPOSITORY https://github.com/facebook/zstd.git
            GIT_TAG v1.5.5
            SOURCE_SUBDIR build/cmake
        )
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(zstd)
        add_library(zstd::libzstd ALIAS libzstd_static)
    else()
        add_library(zstd::libzstd ALIAS zstd::libzstd_static)
    endif()
    
    if(NOT LibArchive_FOUND)
        message(WARNING "libarchive not found. Please install via vcpkg: vcpkg install libarchive:x64-windows zstd:x64-windows")
        message(WARNING "Then configure with: cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake ..")
        if(PkgConfig_FOUND)
            pkg_check_modules(LIBARCHIVE libarchive)
        endif()
    else()
        set(LIBARCHIVE_LIBRARIES LibArchive::LibArchive)
        set(LIBARCHIVE_INCLUDE_DIRS "")
    endif()
else()
    find_package(PkgConfig REQUIRED)
    
    pkg_check_modules(ZSTD REQUIRED libzstd)
    
    add_library(zstd::libzstd INTERFACE IMPORTED)
    target_include_directories(zstd::libzstd INTERFACE ${ZSTD_INCLUDE_DIRS})
    target_link_libraries(zstd::libzstd INTERFACE ${ZSTD_LIBRARIES})
    
    pkg_check_modules(LIBARCHIVE REQUIRED libarchive)
endif()

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
# Sources

file(GLOB SOURCES
  "src/*.cpp"
  "src/*.h"
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(openspm_lib ${SOURCES})
add_executable(openspm main.cpp)

# Version info

set(OPENSPM_VERSION_STRING ${PROJECT_VERSION})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPENSPM_VERSION_STRING "${PROJECT_VERSION}-dev-${GIT_COMMIT_HASH}")
endif()

target_compile_definitions(openspm_lib PRIVATE
  OPENSPM_VERSION="${OPENSPM_VERSION_STRING}"
)

# Include directories

target_include_directories(openspm_lib
  PUBLIC
  ${LIBARCHIVE_INCLUDE_DIRS}
)

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

# Tests

enable_testing()
add_subdirectory(tests)

# Doxygen documentation (Release builds only)

if(OPENSPM_BUILD_DOCS AND CMAKE_BUILD_TYPE STREQUAL "Release")
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
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
else()
  if(NOT OPENSPM_BUILD_DOCS)
    message(STATUS "Doxygen documentation disabled (OPENSPM_BUILD_DOCS=OFF)")
  endif()
endif()

# Packaging

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
