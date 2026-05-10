#pragma once
#include "GameState.h"


class GameOverState : public GameState {
private:
    int finalScore;

public:
    GameOverState(int score);
    virtual ~GameOverState();
    
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();
};
