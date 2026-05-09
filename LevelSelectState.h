#pragma once
#include "GameState.h"
#include "LevelConfig.h"
#include <SFML/Graphics.hpp>

class TextureManager;
class AudioManager;
class GameStateManager;

class LevelSelectState : public GameState {
private:
    TextureManager* texManager;
    AudioManager* audManager;
    GameStateManager* stateManager;
    int finalScore;

    // Menu items
    int selectedLevel;        // 0, 1, or 2
    int maxUnlockedLevel;     // all 3 unlocked after beating campaign

    // Visuals
    sf::Font menuFont;
    sf::Text titleText;
    sf::Text levelTexts[3];
    sf::Text scoreText;
    sf::Text instructionText;

    // Background overlay
    sf::RectangleShape bgOverlay;

    // Level preview rectangles
    sf::RectangleShape levelBoxes[3];

    // Level names
    static const char* LEVEL_NAMES[3];

public:
    LevelSelectState(TextureManager* texMgr, AudioManager* audMgr, int score);
    virtual ~LevelSelectState();

    void setStateManager(GameStateManager* mgr) { this->stateManager = mgr; }

    virtual void update(float dt);
    virtual void render(sf::RenderWindow& window);
    virtual void handleEvent(sf::Event& event);
    virtual void onEnter();
    virtual void onExit();

private:
    void setupText();
    void launchSelectedLevel();
};
