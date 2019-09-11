
file(GLOB_RECURSE EmbeddedSerialFillerTests_SRC
        "*.cpp"
        "*.h")

add_executable(EmbeddedSerialFillerTests ${EmbeddedSerialFillerTests_SRC})

target_link_libraries(EmbeddedSerialFillerTests LINK_PUBLIC EmbeddedSerialFiller gtest)

# The custom target and custom command below allow the unit tests
# to be run.
# If you want them to run automatically by CMake, uncomment #ALL
add_custom_target(
        run_unit_tests #ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSerialFillerTests.touch EmbeddedSerialFillerTests)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSerialFillerTests.touch
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSerialFillerTests)