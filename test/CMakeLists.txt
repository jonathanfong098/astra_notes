add_executable(astra_tests
    test_note_creation.cpp
)
target_link_libraries(astra_tests PRIVATE
    astra_core
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(astra_tests)
