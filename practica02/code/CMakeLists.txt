# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(code C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(code code.c)

pico_set_program_name(code "code")
pico_set_program_version(code "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(code 1)
pico_enable_stdio_usb(code 0)

# Serial por UART0 en GP16 (TX) / GP17 (RX): GP0 y GP1 se usan para el semáforo A
target_compile_definitions(code PRIVATE
        PICO_DEFAULT_UART=0
        PICO_DEFAULT_UART_TX_PIN=16
        PICO_DEFAULT_UART_RX_PIN=17
)

# Add the standard library to the build
target_link_libraries(code
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(code PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(code 
        
        )

pico_add_extra_outputs(code)

