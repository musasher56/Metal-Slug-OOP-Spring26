#include "PausedState.h"

PausedState::PausedState() {
    this->id = GSTATE_PAUSED;
}

PausedState::~PausedState() {
    
}

void PausedState::update(float dt) {
    
}

void PausedState::render(RenderWindow& window) {
    
    
}

void PausedState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Escape || event.key.code == Keyboard::P) {
            
            
        }
    }
}

void PausedState::onEnter() {
    
}

void PausedState::onExit() {
    
}
