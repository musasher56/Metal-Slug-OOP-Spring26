#pragma once
#include "Constants.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "GameStateManager.h"
#include "MenuState.h"
#include "CharSelectState.h"
#include "PlayState.h"
#include "Level.h"

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    RenderWindow window;
    GameStateManager* stateManager;
    TextureManager* texManager;
    AudioManager* audManager;

    int gameMode;
    int selectedChar;   // 0=Marco, 1=Tarma, 2=Eri, 3=Fio (set by CharSelectState)
    bool running;

    void initialize();
    void handleEvents();
    void update(float dt);
    void render();
    void cleanup();
};