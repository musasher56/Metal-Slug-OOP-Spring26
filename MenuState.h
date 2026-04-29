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
    
    virtual void update(float dt) override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event) override;
    virtual void onEnter() override;
    virtual void onExit() override;
    
    int getSelectedMode() const;
};
