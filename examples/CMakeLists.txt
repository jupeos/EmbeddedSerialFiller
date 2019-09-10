
#file(GLOB_RECURSE SerialFillerExample_SRC
#        "*.cpp"
#        "*.hpp"
#        )

add_executable(BasicExample BasicExample.cpp)
target_link_libraries(BasicExample LINK_PUBLIC SerialFiller)