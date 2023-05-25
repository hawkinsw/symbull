cmake_minimum_required(VERSION 3.15)

project(MotivatingExample VERSION 1.0 DESCRIPTION "A motivating example of why we need this tool.")
set(CMAKE_CXX_STANDARD 20)

add_executable(eg eg.cpp)
add_custom_target(Copyeg ALL DEPENDS "${CMAKE_BINARY_DIR}/eg")
add_custom_target(Stripeg ALL DEPENDS "${CMAKE_BINARY_DIR}/egs")
add_custom_target(Objdumpeg ALL DEPENDS "${CMAKE_BINARY_DIR}/egs.obj" "${CMAKE_BINARY_DIR}/eg.obj")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/eg" COMMAND cp "$<TARGET_FILE:eg>" "${CMAKE_BINARY_DIR}/eg")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/egs" COMMAND cp "$<TARGET_FILE:eg>" "${CMAKE_BINARY_DIR}/egs" COMMAND strip "${CMAKE_BINARY_DIR}/egs")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/eg.obj" COMMAND objdump -Cd "$<TARGET_FILE:eg>" > "${CMAKE_BINARY_DIR}/eg.obj")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/egs.obj" COMMAND objdump -Cd "${CMAKE_BINARY_DIR}/egs" > "${CMAKE_BINARY_DIR}/egs.obj")
