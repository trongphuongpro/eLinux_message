cmake_minimum_required(VERSION 3.0)

#-----------------------------------------------------------------------------#
find_package(Threads REQUIRED)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_COMPILER arm-linux-g++)
set(CMAKE_C_COMPILER arm-linux-gcc)
#-----------------------------------------------------------------------------#

add_subdirectory(/home/trongphuong/Work/BBB/libraries/uart uart)
add_subdirectory(/home/trongphuong/Work/BBB/libraries/crc32 crc32)

set(TARGET testbench)

add_executable(${TARGET} test.cpp ../src/message.cpp ../src/message_uart.cpp)
target_link_libraries(${TARGET} uart crc32)
target_include_directories(${TARGET} PUBLIC ../include)

#-----------------------------------------------------------------------------#
target_compile_options(${TARGET} PUBLIC 
										-Wall
										-Werror
										-O2
)
#-----------------------------------------------------------------------------#
install(TARGETS ${TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)