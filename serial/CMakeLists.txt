
cmake_minimum_required(VERSION 3.7.2)

project(serial C)

#set(CMAKE_C_STANDARD 11)
#
#SET(CMAKE_BUILD_TYPE Debug)
#
#set(GCC_COMPILE_FLAGS "-Wall -Werror -Wvla")

add_library(lib_bit src/bit.c)
target_include_directories(lib_bit PUBLIC include/)

add_library(lib_serial_parity src/serial_parity.c)
target_include_directories(lib_serial_parity PUBLIC include/)

add_library(lib_imx_uart_regs src/imx_uart_regs.c)
target_include_directories(lib_imx_uart_regs PUBLIC include/)
target_link_libraries(
        lib_imx_uart_regs
        lib_sel4cp_imx8mm
        lib_serial_parity
        lib_bit
)

add_library(lib_imx_uart src/imx_uart.c)
target_include_directories(lib_imx_uart PUBLIC include/)
target_link_libraries(
        lib_imx_uart
        lib_sel4cp_imx8mm
        lib_imx_uart_regs
)

add_library(lib_serial_driver src/serial_driver.c)
target_include_directories(lib_serial_driver PUBLIC include/)
target_link_libraries(
        lib_serial_driver
        lib_sel4cp_imx8mm
        lib_imx_uart
)

add_library(lib_serial_client src/serial_client.c)
target_include_directories(lib_serial_client PUBLIC include/)
target_link_libraries(
        lib_serial_client
        lib_sel4cp_imx8mm
)


