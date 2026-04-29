#include "PausedState.h"

PausedState::PausedState() {
    this->id = GSTATE_PAUSED;
}

PausedState::~PausedState() {
    // WHY: PausedState has no dynamic resources
}

void PausedState::update(float dt) {
    // WHY: Pause state doesn't update game logic
}

void PausedState::render(RenderWindow& window) {
    // WHY: Render pause overlay (semi-transparent)
    // TODO: Implement pause menu rendering
}

void PausedState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Escape || event.key.code == Keyboard::P) {
            // WHY: Signal to pop pause state and resume game
            // Handled by Game class via return value or callback
        }
    }
}

void PausedState::onEnter() {
    // WHY: Pause the game when entering
}

void PausedState::onExit() {
    // WHY: Resume the game when exiting
}
