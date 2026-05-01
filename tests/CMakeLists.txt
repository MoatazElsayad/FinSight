add_executable(finsight_tests backend_tests.cpp)
target_include_directories(finsight_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(finsight_tests PRIVATE
    finsight_core
    GTest::gtest_main
    GTest::gmock
)
include(GoogleTest)
gtest_discover_tests(finsight_tests)
