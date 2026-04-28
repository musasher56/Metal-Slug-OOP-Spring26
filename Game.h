#pragma once
#include "Constants.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "GameStateManager.h"
#include "MenuState.h"
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
    bool running;

    void initialize();
    void handleEvents();
    void update(float dt);
    void render();
    void cleanup();
};