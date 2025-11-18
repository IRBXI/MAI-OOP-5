project(Tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.0
  TLS_VERIFY false
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit test_main.cpp pool_allocator.cpp)
target_link_libraries(unit gtest_main allocator)
