cmake_minimum_required(VERSION 3.5)
project(rtk_test)

enable_testing()

add_compile_options(-fPIC -shared -rdynamic -std=c++17 -ldl)

if (DEFINED ENV{BUILD_OPTIMIZE_LEVEL}) 
    set(CMAKE_CXX_FLAGS "-O$ENV{BUILD_OPTIMIZE_LEVEL} -pthread -Wall")
else()
    set(CMAKE_CXX_FLAGS "-O3 -pthread")
endif()

include_directories(include)

link_directories(lib)


aux_source_directory(. src)
add_executable(rtk_test ${src})
target_link_libraries(rtk_test rtkpps)
