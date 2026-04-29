#pragma once
#include "Constants.h"
#include "Vector2d.h"

// WHY: GameState is the abstract base for all game states (Menu, Play, Paused, etc.)
// State pattern allows stack-based state management (pause overlay, etc.)
class GameState {
protected:
    int id;  // GameStateID constant
    int gameMode;  // Store selected game mode for state transitions

public:
    GameState();
    virtual ~GameState();
    
    virtual void update(float dt) = 0;
    virtual void render(RenderWindow& window) = 0;
    virtual void handleEvent(Event& event) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    
    int getID() const;
};
