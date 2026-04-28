#pragma once
#include "GameState.h"

// WHY: PausedState handles the pause overlay during gameplay
class PausedState : public GameState {
public:
    PausedState();
    virtual ~PausedState();
    
    virtual void update(float dt) override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event) override;
    virtual void onEnter() override;
    virtual void onExit() override;
};
