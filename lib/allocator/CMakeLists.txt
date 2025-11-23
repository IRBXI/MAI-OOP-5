project(Allocator)

set(HEADERS pool_allocator.hpp)
add_library(allocator ${HEADERS})

set_target_properties(allocator PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(allocator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
