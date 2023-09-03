cmake_minimum_required(VERSION 3.5)
project(rtk_test)

add_compile_options(-fPIC -shared -rdynamic  -ldl)

if (DEFINED ENV{BUILD_OPTIMIZE_LEVEL}) 
    set(CMAKE_CXX_FLAGS "-O$ENV{BUILD_OPTIMIZE_LEVEL} -pthread -Wall")
else()
    set(CMAKE_CXX_FLAGS "-O3 -pthread")
endif()

add_subdirectory(src)

include_directories(src)

aux_source_directory(RTK_Test SRC_DIR_LIST)

add_executable(rtk_test ${SRC_DIR_LIST})

target_link_libraries(rtk_test rtkpps imucamera)

set(INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../install/${PLATFORM_NAME})

install(TARGETS rtk_test DESTINATION ${INSTALL_PATH}/bin)
