#include "GameState.h"

GameState::~GameState() {
    // WHY: Virtual destructor for proper cleanup of derived states
}

int GameState::getID() const {
    return this->id;
}
