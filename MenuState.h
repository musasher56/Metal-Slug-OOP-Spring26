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

    
    
    virtual GameState* createNextState();
};
