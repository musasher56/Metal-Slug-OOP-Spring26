#pragma once
#include "Constants.h"
#include "Vector2d.h"

class TextureManager;
class AudioManager;
class GameStateManager;


class GameState {
public:
    
    int gameMode;
    int selectedLevel;    
    int selectedChar;     

    TextureManager* texManager;
    AudioManager* audManager;
    GameStateManager* stateManager;

    bool shouldGoBack;    
    bool shouldExit;      

protected:
    int id;

public:
    GameState();
    virtual ~GameState();

    virtual void update(float dt) = 0;
    virtual void render(RenderWindow& window) = 0;
    virtual void handleEvent(Event& event) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;

    
    
    
    virtual GameState* createNextState();

    int getID() const;

    bool getShouldGoBack() const { return this->shouldGoBack; }
    bool getShouldExit() const { return this->shouldExit; }

    void setStateManager(GameStateManager* mgr) { this->stateManager = mgr; }
};
