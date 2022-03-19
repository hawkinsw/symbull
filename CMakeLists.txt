cmake_minimum_required(VERSION 3.15)


project(MachineLearningELF VERSION 1.0 DESCRIPTION "Machine Learning on ELF files")
set(CMAKE_CXX_STANDARD 20)



set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(LibElf REQUIRED)

include_directories("include/")
include_directories("deps/args/")
include_directories(${LIBELF_INCLUDE_DIRS})
add_executable(learn_elf main.cpp symbol.cpp)
target_link_libraries(learn_elf PRIVATE elf)
target_link_libraries(learn_elf PRIVATE ${DEFINES})
