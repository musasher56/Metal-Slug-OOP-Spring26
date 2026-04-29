#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "MainMenu.h"

// WHY: MenuState handles the main menu screen with game mode selection
class MenuState : public GameState {
private:
    MainMenu* mainMenu;
    TextureManager* texManager;
    AudioManager* audManager;

public:
    MenuState(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~MenuState();
    
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();
    
    int getSelectedMode() const;
};
