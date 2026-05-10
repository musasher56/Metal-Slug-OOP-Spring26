#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"


class MainMenu;

class MenuState : public GameState {
private:
    MainMenu* mainMenu;

public:
    MenuState(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~MenuState();

    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();

    // Polymorphic transition: when the player has chosen both a mode and a
    // level, this returns a new CharSelectState. Otherwise returns nullptr.
    virtual GameState* createNextState();
};
