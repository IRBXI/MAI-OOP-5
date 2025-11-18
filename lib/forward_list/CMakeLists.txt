project(ForwardList)


set(HEADERS forward_list.hpp)
add_library(forward_list ${HEADERS})

set_target_properties(forward_list PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(forward_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
