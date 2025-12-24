# tests/CMakeLists.txt

file(GLOB TEST_SOURCES "*.cpp")

add_executable(openspm_tests ${TEST_SOURCES})

target_link_libraries(openspm_tests
  PRIVATE
    yaml-cpp::yaml-cpp
    httplib::httplib
    openspm_lib
)

target_compile_definitions(openspm_tests PRIVATE
  OPENSPM_VERSION="${OPENSPM_VERSION_STRING}"
)

add_test(
  NAME openspm.unit
  COMMAND openspm_tests
)
