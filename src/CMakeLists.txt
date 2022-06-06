

file(GLOB_RECURSE embedded-serial-filler_SRC "*.cpp")

list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/EmbeddedSerialFiller_NoRTOS.cpp)
list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/EmbeddedSerialFiller_RTOS.cpp)

if(ETL_PROFILE MATCHES "PROFILE_EMBOS")
	message("ETL_PROFILE detected to be PROFILE_EMBOS")
	list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/esf_freertos_abstraction.cpp)
elseif(ETL_PROFILE MATCHES "PROFILE_FREERTOS")
	message("ETL_PROFILE detected to be PROFILE_EMBOS")
	list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/esf_embos_abstraction.cpp)
else ()
	list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/esf_embos_abstraction.cpp)
	list(REMOVE_ITEM embedded-serial-filler_SRC ${CMAKE_CURRENT_SOURCE_DIR}/esf_freertos_abstraction.cpp)
	message(${embedded-serial-filler_SRC})
endif ()

file(GLOB_RECURSE embedded-serial-filler_HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

add_library(EmbeddedSerialFiller ${embedded-serial-filler_SRC} ${embedded-serial-filler_HEADERS})

target_include_directories(EmbeddedSerialFiller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# On Linux, "sudo make install" will typically copy the library libEmbeddedSerialFiller.a
# into the folder /usr/local/bin
install(TARGETS EmbeddedSerialFiller DESTINATION lib)

# On Linux, "sudo make install" will typically copy the
# folder into /usr/local/include
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/EmbeddedSerialFiller DESTINATION include)