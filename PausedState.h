#pragma once
#include "GameState.h"

// WHY: PausedState handles the pause overlay during gameplay
class PausedState : public GameState {
public:
    PausedState();
    virtual ~PausedState();
    
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();
};
