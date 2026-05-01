cmake_minimum_required(VERSION 3.16)
project(MetalSlug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SFML location ──────────────────────────────────────────────────────────────
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

# ── Sources ────────────────────────────────────────────────────────────────────
set(SOURCES
        main.cpp
        AimController.cpp
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
        PlayerSoldier.cpp
        TransformationState.cpp
        Gravity.cpp
        Weapon.cpp
        Grenade.cpp
        Projectile.cpp
        Collectible.cpp
        CharacterManager.cpp
        LevelManager.cpp
        ScoreManager.cpp
        HUD.cpp
        Vehicle.cpp
        ProjectileManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# ── Output goes to project root (next to resources/) ──────────────────────────
# This fixes "Unable to open file" — executable now runs where resources/ lives
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# ── Link & include ─────────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# ── Platform-specific ─────────────────────────────────────────────────────────
if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

# On Windows: copy SFML DLLs next to the exe
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_FILE_DIR:sfml-graphics>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# ── Resources check ───────────────────────────────────────────────────────────
# No copy needed — exe now lives in project root where resources/ already exists
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/resources")
    message(WARNING "Resources folder not found: ${CMAKE_SOURCE_DIR}/resources")
endif()