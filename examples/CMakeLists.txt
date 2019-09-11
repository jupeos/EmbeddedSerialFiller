
#file(GLOB_RECURSE EmbeddedSerialFillerExample_SRC
#        "*.cpp"
#        "*.h"
#        )

add_executable(BasicExample BasicExample.cpp)
target_link_libraries(BasicExample LINK_PUBLIC EmbeddedSerialFiller)