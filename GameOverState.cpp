#include "GameOverState.h"

GameOverState::GameOverState(int score) 
    : finalScore(score) {
    this->id = GSTATE_GAME_OVER;
}

GameOverState::~GameOverState() {
    // WHY: GameOverState has no dynamic resources
}

void GameOverState::update(float dt) {
    // WHY: Game over state doesn't update game logic
}

void GameOverState::render(RenderWindow& window) {
    // WHY: Render game over screen with final score
    // TODO: Implement game over rendering
    window.clear(Color::Black);
}

void GameOverState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Enter) {
            // WHY: Restart game or return to menu
        } else if (event.key.code == Keyboard::Escape) {
            // WHY: Return to menu
        }
    }
}

void GameOverState::onEnter() {
    // WHY: Initialize game over state
}

void GameOverState::onExit() {
    // WHY: Cleanup game over state
}
