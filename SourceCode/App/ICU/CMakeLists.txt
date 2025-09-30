#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(optimus_icu)

# 定义 common 目录
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)

# 定义 driver 目录
set(DRIVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/driver)

# 检查 common 目录是否存在
if(NOT EXISTS ${COMMON_DIR})
  message(FATAL_ERROR "The directory '${COMMON_DIR}' does not exist. Check your project structure.")
endif()
set(CMAKE_CXX_STANDARD 11) 
# 添加 main 应用程序源文件
target_sources(app PRIVATE
  src/main.c
  src/nx_crc.c
  src/dfu.c
  src/info.c
  src/sensor.c
  src/rtc.c
  src/events.c
  src/fuel_gauge_monitor.c
  src/thermal_monitor.c
  ${COMMON_DIR}/G3_ProtocolImplBLEPeripheral.c
  ${COMMON_DIR}/G3_ProtocolImplUart.c
  ${COMMON_DIR}/G3_Seq.c
  ${DRIVER_DIR}/bsp_gpio.c
  ${DRIVER_DIR}/bsp_ice40.c
  ${DRIVER_DIR}/bsp_gd25q32.c
  ${DRIVER_DIR}/bsp_tmp119.c
  ${DRIVER_DIR}/bq27427/bq274xx.c
)

# 递归包含 common 目录和子目录
function(get_all_subdirectories BASE_DIR OUT_VAR)
    file(GLOB_RECURSE SUB_DIRS LIST_DIRECTORIES true "${BASE_DIR}/*")
    set(FILTERED_DIRS "")
    foreach(SUB_DIR ${SUB_DIRS})
        if (IS_DIRECTORY ${SUB_DIR})
            list(APPEND FILTERED_DIRS ${SUB_DIR})
        endif()
    endforeach()
    set(${OUT_VAR} ${FILTERED_DIRS} PARENT_SCOPE)
endfunction()

# 获取 common 目录的所有子目录
get_all_subdirectories(${COMMON_DIR} COMMON_SUBDIRS)

# 添加到包含路径
zephyr_include_directories(${COMMON_DIR})
zephyr_include_directories(${COMMON_SUBDIRS})

# 条件添加 UART ASYNC API
target_sources_ifdef(CONFIG_BT_NUS_UART_ASYNC_ADAPTER app PRIVATE
  src/uart_async_adapter.c
)

# 添加库头文件
zephyr_library_include_directories(.)
