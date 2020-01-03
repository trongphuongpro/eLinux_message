cmake_minimum_required(VERSION 3.0)

#-----------------------------------------------------------------------------#
find_package(Threads REQUIRED)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_COMPILER arm-linux-g++)
set(CMAKE_C_COMPILER arm-linux-gcc)
#-----------------------------------------------------------------------------#

add_subdirectory(/home/trongphuong/Work/BBB/libraries/uart uart)

set(TARGET testbench)

add_executable(${TARGET} test.cpp ../src/message.cpp)
target_link_libraries(${TARGET} uart ${CMAKE_SOURCE_DIR}/../lib/libcrc32.a)
target_include_directories(${TARGET} PUBLIC ../include)

#-----------------------------------------------------------------------------#
target_compile_options(${TARGET} PUBLIC -std=gnu++11
										-Wall
										-Werror
										-O2
)
#-----------------------------------------------------------------------------#
install(TARGETS ${TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)