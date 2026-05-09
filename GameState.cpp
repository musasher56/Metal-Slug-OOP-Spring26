#include "GameState.h"

GameState::GameState()
    : id(-1)
    , gameMode(MODE_SURVIVAL)
    , selectedLevel(0)
    , selectedChar(0)
    , texManager(nullptr)
    , audManager(nullptr)
    , stateManager(nullptr)
    , shouldGoBack(false)
    , shouldExit(false)
{
}

GameState::~GameState() {
}

int GameState::getID() const {
    return this->id;
}

// Default: no transition. Subclasses override when they're ready to advance.
GameState* GameState::createNextState() {
    return nullptr;
}
