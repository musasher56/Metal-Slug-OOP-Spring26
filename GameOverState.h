#pragma once
#include "GameState.h"

// WHY: GameOverState handles the game over screen with final score display
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
