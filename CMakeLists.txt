cmake_minimum_required(VERSION 3.16)
project(MetalSlug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    if(EXISTS "/opt/homebrew/lib/cmake/SFML")
        set(SFML_DIR "/opt/homebrew/lib/cmake/SFML")
    elseif(EXISTS "/usr/local/lib/cmake/SFML")
        set(SFML_DIR "/usr/local/lib/cmake/SFML")
    endif()
elseif(WIN32)
    if(NOT DEFINED SFML_DIR AND NOT DEFINED ENV{SFML_DIR})
        message(WARNING "SFML_DIR not set. Pass -DSFML_DIR=<path>")

    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_WIN32_EXECUTABLE TRUE)
    endif()
endif()

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(SOURCES
        main.cpp
        Game.cpp
        GameState.cpp
        GameStateManager.cpp
        MenuState.cpp
        PlayState.cpp
        PausedState.cpp
        GameOverState.cpp
        TextureManager.cpp
        AudioManager.cpp
        Level.cpp
        Vector2d.cpp
        Animation.cpp
        MainMenu.cpp
        Entity.cpp
        DamagableEntity.cpp
        Soldier.cpp
        Player.cpp
        TransformationState.cpp
        NormalState.cpp
        UndeadState.cpp
        MummyState.cpp
        AimController.cpp
        Gravity.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_FILE_DIR:sfml-graphics>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/resources")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    )
else()
    message(WARNING "Resources folder not found: ${CMAKE_SOURCE_DIR}/resources")
endif()