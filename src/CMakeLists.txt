

file(GLOB_RECURSE embedded-serial-filler_SRC
        "*.cpp")

file(GLOB_RECURSE embedded-serial-filler_HEADERS
        "${CMAKE_SOURCE_DIR}/include/*.h")

add_library(EmbeddedSerialFiller ${embedded-serial-filler_SRC} ${embedded-serial-filler_HEADERS})

target_include_directories(EmbeddedSerialFiller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# On Linux, "sudo make install" will typically copy the library libEmbeddedSerialFiller.a
# into the folder /usr/local/bin
install(TARGETS EmbeddedSerialFiller DESTINATION lib)

# On Linux, "sudo make install" will typically copy the
# folder into /usr/local/include
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/EmbeddedSerialFiller DESTINATION include)