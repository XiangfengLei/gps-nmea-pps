cmake_minimum_required(VERSION 3.5)
project(rtkpps)

add_compile_options(-fPIC -shared -rdynamic  -ldl)

if (DEFINED ENV{BUILD_OPTIMIZE_LEVEL}) 
    set(CMAKE_CXX_FLAGS "-O$ENV{BUILD_OPTIMIZE_LEVEL} -pthread -Wall")
else()
    set(CMAKE_CXX_FLAGS "-O3 -pthread")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../ImuCamera/video)

add_library(rtkpps SHARED rtkpps.cpp)

target_link_libraries(rtkpps imucamera)

set(INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../install/${PLATFORM_NAME})

install(TARGETS rtkpps DESTINATION ${INSTALL_PATH}/lib)
install(FILES rtkpps.h DESTINATION ${INSTALL_PATH}/include)
