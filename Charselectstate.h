#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include <SFML/Graphics.hpp>

class CharSelectState : public GameState {

    private:
    TextureManager* texManager;
    AudioManager* audManager;
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded;
    sf::RectangleShape highlightBox;
    sf::Font  font;
    bool fontLoaded;
    int hoveredChar;
    static const char* CHAR_NAMES[4];
    void buildSlotPositions();
    sf::FloatRect slotRects[4];

public:
    CharSelectState(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~CharSelectState();
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(sf::Event& event);
    virtual void onEnter();
    virtual void onExit();
    virtual GameState* createNextState();

};
