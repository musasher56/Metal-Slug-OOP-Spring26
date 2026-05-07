#include "GameOverState.h"

GameOverState::GameOverState(int score) 
    : finalScore(score) {
    this->id = GSTATE_GAME_OVER;
}

GameOverState::~GameOverState() {
    
}

void GameOverState::update(float dt) {
    
}

void GameOverState::render(RenderWindow& window) {
    
    
    window.clear(Color::Black);
}

void GameOverState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Enter) {
            
        } else if (event.key.code == Keyboard::Escape) {
            
        }
    }
}

void GameOverState::onEnter() {
    
}

void GameOverState::onExit() {
    
}
