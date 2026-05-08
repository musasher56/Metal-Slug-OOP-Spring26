#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include <SFML/Graphics.hpp>

// ============================================================================
//  CharSelectState
//  Sits between the main menu (mode select) and PlayState in the state stack.
//  Shows the "SOLDIER SELECT" screen and waits for the player to press 1-4.
//  Game.cpp polls getSelectedChar() each frame; when it returns 0-3 it pops
//  this state and pushes PlayState with that character index.
//
//  Image expected at: resources/Sprites/CharacterSelect.png
//  (Place the Characterselect.png you have there — rename to match exactly.)
// ============================================================================

class CharSelectState : public GameState {
private:
    TextureManager* texManager;
    AudioManager*   audManager;

    // The full-screen background image (the soldier select art)
    sf::Texture     bgTexture;
    sf::Sprite      bgSprite;
    bool            bgLoaded;

    // Highlight box drawn over the currently hovered character slot
    sf::RectangleShape highlightBox;

    // Font + text for "press 1-4" hint and character names
    sf::Font  font;
    bool      fontLoaded;

    // Which slot is currently highlighted (0=Marco, 1=Eri, 2=Tarma, 3=Fio)
    // Keyboard arrow keys cycle through slots; Enter or 1-4 confirms.
    int hoveredChar;

    // Set to 0-3 once the player confirms selection; -1 means "not yet chosen".
    // Game.cpp reads this via getSelectedChar().
    int selectedChar;

    // Four name labels for the slots — char arrays, no std::string
    // Order matches the image: Marco(1), Eri(2), Tarma(3), Fio(4)
    static const char* CHAR_NAMES[4];

public:
    CharSelectState(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~CharSelectState();

    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();

    // Returns 0-3 once player has confirmed a character, -1 otherwise.
    int getSelectedChar() const;

private:
    // Positions of the four portrait slots in the image (approximate screen coords
    // when the image is scaled to 1280x720). Adjust if your image differs.
    void buildSlotPositions();
    sf::FloatRect slotRects[4];   // Bounding boxes for the four portrait panels
};