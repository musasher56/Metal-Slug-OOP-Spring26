#pragma once
#include "GameState.h"

// WHY: GameOverState handles the game over screen with final score display
class GameOverState : public GameState {
private:
    int finalScore;

public:
    GameOverState(int score);
    virtual ~GameOverState();
    
    virtual void update(float dt) override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event) override;
    virtual void onEnter() override;
    virtual void onExit() override;
};
