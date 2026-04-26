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
    RenderWindow    window;
    TextureManager* texManager;
    AudioManager*   audManager;
    MainMenu*       mainMenu;
    Level*          level;
    Player*         player;

    sf::Texture bgTex;
    sf::Sprite  bgSprite;

    int  gameMode;
    bool running;
    bool inMenu;

    // WHY: track key state via events — isKeyPressed unreliable on macOS
    bool movingLeft;
    bool movingRight;

    void initialize();
    void startGame();
    void handleEvents();
    void update(float dt);
    void render();
    void cleanup();
};