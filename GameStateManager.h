#pragma once
#include "GameState.h"



class GameStateManager {
private:
    GameState** stateStack;  
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
