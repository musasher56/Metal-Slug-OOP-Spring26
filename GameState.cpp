#include "GameState.h"

GameState::GameState() : id(-1), gameMode(MODE_SURVIVAL) {
    // WHY: Initialize base state with default values
}

GameState::~GameState() {
    // WHY: Virtual destructor for proper cleanup of derived states
}

int GameState::getID() const {
    return this->id;
}
