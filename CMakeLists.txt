cmake_minimum_required(VERSION 3.16)
project(MetalSlug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─────────────────────────────────────────────
# SFML PATH DETECTION (platform agnostic)
# WHY: each OS puts SFML in a different location
# ─────────────────────────────────────────────
if(APPLE)
    # Apple Silicon (M1/M2/M3/M4) Homebrew path
    if(EXISTS "/opt/homebrew/lib/cmake/SFML")
        set(SFML_DIR "/opt/homebrew/lib/cmake/SFML")
        # Intel Mac Homebrew / manual install path
    elseif(EXISTS "/usr/local/lib/cmake/SFML")
        set(SFML_DIR "/usr/local/lib/cmake/SFML")
    endif()
elseif(WIN32)
    # WHY: Windows users set this via cmake -DSFML_DIR="C:/SFML/lib/cmake/SFML"
    # or via environment variable. We just warn if not set.
    if(NOT DEFINED SFML_DIR AND NOT DEFINED ENV{SFML_DIR})
        message(WARNING "SFML_DIR not set on Windows. Pass -DSFML_DIR=<path to SFML/lib/cmake/SFML>")
    endif()
    # WHY: prevent console window popup on Windows release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_WIN32_EXECUTABLE TRUE)
    endif()
elseif(UNIX)
    # WHY: Linux installs SFML via apt/pacman to standard system paths
    # find_package will locate it automatically — no manual path needed
endif()

# ─────────────────────────────────────────────
# FIND SFML
# ─────────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# ─────────────────────────────────────────────
# SOURCE FILES
# ─────────────────────────────────────────────
set(SOURCES
        main.cpp
        Game.cpp
        MainMenu.cpp
        TextureManager.cpp
        AudioManager.cpp
        Level.cpp
        Character.cpp
        Player.cpp
)

# ─────────────────────────────────────────────
# EXECUTABLE
# ─────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

# ─────────────────────────────────────────────
# LINK SFML
# ─────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

# ─────────────────────────────────────────────
# INCLUDE DIRS
# ─────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# ─────────────────────────────────────────────
# PLATFORM-SPECIFIC COMPILE OPTIONS
# ─────────────────────────────────────────────
if(APPLE)
    # WHY: suppress harmless SFML char_traits deprecation warnings on newer macOS SDKs
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

if(WIN32)
    # WHY: SFML needs this on MinGW to link correctly
    target_compile_definitions(${PROJECT_NAME} PRIVATE SFML_STATIC)
endif()

# ─────────────────────────────────────────────
# COPY RESOURCES TO BUILD DIR
# WHY: game loads assets relative to executable — must exist next to it
# ─────────────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
        COMMENT "Copying resources folder to build directory"
)

# ─────────────────────────────────────────────
# WINDOWS: copy SFML DLLs next to executable
# WHY: on Windows SFML is dynamic by default — DLLs must be next to .exe
# ─────────────────────────────────────────────
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_FILE_DIR:sfml-graphics>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying SFML DLLs to build directory"
    )
endif()