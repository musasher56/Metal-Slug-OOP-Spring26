#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include <SFML/Graphics.hpp>

// ============================================================================
//  CharSelectState
//  Player picks their soldier (Marco/Tarma/Eri/Fio).
//  When selection is confirmed, createNextState() returns a PlayState
//  initialised with the game mode, selected level, and chosen character
//  that were accumulated through the menu chain.
// ============================================================================

class CharSelectState : public GameState {
private:
    TextureManager* texManager;
    AudioManager* audManager;

    sf::Texture     bgTexture;
    sf::Sprite      bgSprite;
    bool            bgLoaded;

    sf::RectangleShape highlightBox;

    sf::Font  font;
    bool      fontLoaded;

    int hoveredChar;

    static const char* CHAR_NAMES[4];

public:
    CharSelectState(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~CharSelectState();

    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(sf::Event& event);
    virtual void onEnter();
    virtual void onExit();

    // Polymorphic transition: once a character is chosen, returns PlayState
    virtual GameState* createNextState();

private:
    void buildSlotPositions();
    sf::FloatRect slotRects[4];
};
