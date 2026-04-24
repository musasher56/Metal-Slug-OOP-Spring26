#pragma once
#include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;

// Screen settings
const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int FRAMERATE_LIMIT = 60;

// Game modes
const int MODE_SURVIVAL = 0;
const int MODE_CAMPAIGN = 1;
const int MODE_SELF_PLAY = 2;

// Limits
const int MAX_SOUNDS = 50;
const int MAX_TEXTURES = 100;
const int MAX_NAME_LEN = 64;