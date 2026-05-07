#pragma once
#include "Constants.h"
#include "Vector2d.h"



class GameState {
protected:
    int id;  
    int gameMode;  

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
