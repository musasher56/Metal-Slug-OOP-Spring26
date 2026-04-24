#pragma once
#include "Constants.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "MainMenu.h"
#include "Level.h"
#include "Player.h"

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    RenderWindow window;
    TextureManager* texManager;
    AudioManager*   audManager;
    MainMenu*       mainMenu;
    Level*          level;
    Player*         player;

    int   gameMode;
    bool  running;
    bool  inMenu;

    // WHY: track movement via events — isKeyPressed broken on macOS + SFML 2
    bool  movingLeft;
    bool  movingRight;

    void initialize();
    void handleEvents();
    void update(float dt);
    void render();
    void cleanup();
    void startGame();
};