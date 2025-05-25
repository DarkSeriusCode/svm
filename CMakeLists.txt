cmake_minimum_required(VERSION 3.15)
project(svm LANGUAGES C)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(COMPILE_CMDS_PATH "${PROJECT_SOURCE_DIR}/compile_commands.json")

# -------- Common --------
file(GLOB COMMON_SRC "common/*.c")
add_library(common ${COMMON_SRC})

# -------- Sasm --------
file(GLOB SASM_SRC "assembler/*.c")
add_executable(sasm ${SASM_SRC})
target_link_libraries(sasm PUBLIC common)

# -------- Svm --------
file(GLOB SVM_SRC "vm/*.c")
add_executable(svm ${SVM_SRC})
target_link_libraries(svm PUBLIC common)

# -------- Devices --------
file(GLOB DEV_FILES "dev/*.c")
foreach(file ${DEV_FILES})
    get_filename_component(dev_name ${file} NAME_WE)
    add_library(${dev_name} MODULE ${file})
    list(APPEND DEVICES ${dev_name})
endforeach()

# -------- Crap --------
include_directories(".")
