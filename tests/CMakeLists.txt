include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(ColAndreas_tests
    stubs.cpp
    test_math.cpp
    test_removedbuilding.cpp
    test_modelref.cpp
    test_dbreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/DynamicWorld.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ColObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ColAndreasDatabaseReader.cpp
)

target_include_directories(ColAndreas_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/SDK
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/SDK/amx
    ${BULLET_INCLUDE_DIR}
)

target_link_libraries(ColAndreas_tests
    GTest::gtest_main
    ${BULLET_LIBRARIES}
)

add_test(NAME ColAndreas_tests COMMAND ColAndreas_tests)
