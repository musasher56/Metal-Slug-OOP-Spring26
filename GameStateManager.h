#pragma once
#include "GameState.h"

// WHY: GameStateManager manages the stack of GameStates (Menu, Play, Paused, etc.)
// Stack-based approach allows pause overlay without losing game state
class GameStateManager {
private:
    GameState** stateStack;  // WHY: Manual array to avoid <vector>
    int stackSize;
    int capacity;

public:
    GameStateManager();
    ~GameStateManager();
    
    void push(GameState* state);
    void pop();
    GameState* peek();
    
    void update(float dt);
    void render(RenderWindow& window);
    void handleEvent(Event& event);
    void changeState(GameState* newState);
};
