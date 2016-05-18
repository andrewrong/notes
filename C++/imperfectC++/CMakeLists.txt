cmake_minimum_required(VERSION 3.0)
project(ImperfectC++)

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

#SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
#INCLUDE(EnableCoverageReport)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated")
if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()

set (CMAKE_INSTALL_RPATH ./lib)
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

include_directories(${PROJECT_SOURCE_DIR}/src)

# 创建bin 和 lib目录
MAKE_DIRECTORY(${PROJECT_SOURCE_DIR}/lib)
MAKE_DIRECTORY(${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#注意：不是很可靠。可能不会自动更新，需要重新刷下本文件
FILE(GLOB_RECURSE SRC_LIST RELATIVE ${PROJECT_SOURCE_DIR} "src/*.c*")
FILE(GLOB_RECURSE TEST_SRC_LIST RELATIVE ${PROJECT_SOURCE_DIR} "test/*.c*")
FILE(GLOB_RECURSE SRC_INCLUDE_LIST RELATIVE ${PROJECT_SOURCE_DIR} "src/*.h")

# 设置可执行文件的名字和编译的文件和头文件
add_executable(ImperfectC++ main.cpp ${SRC_LIST} ${SRC_INCLUDE_LIST})
target_link_libraries(ImperfectC++ ${CONAN_LIBS})

#test
#add_executable(testImperfectC++ ${SRC_LIST} ${TEST_SRC_LIST})
#target_link_libraries(testImperfectC++ ${CONAN_LIBS})

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    target_link_libraries(ImperfectC++ rt pthread libjemalloc.a)
#    target_link_libraries(testImperfectC++ rt pthread)
endif()

