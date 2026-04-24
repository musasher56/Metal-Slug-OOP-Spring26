cmake_minimum_required(VERSION 3.16)
project(MetalSlug)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

if(APPLE)
    set(SFML_DIR "/usr/local/lib/cmake/SFML")
elseif(WIN32)
    if(NOT DEFINED SFML_DIR)
        message(WARNING "SFML_DIR not set. Please pass -DSFML_DIR=<path>")
    endif()
endif()

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)

# Auto-copy resources folder to build directory every build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)
