#include "GameState.h"

GameState::GameState() : id(-1), gameMode(MODE_SURVIVAL) {
    
}

GameState::~GameState() {
    
}

int GameState::getID() const {
    return this->id;
}
