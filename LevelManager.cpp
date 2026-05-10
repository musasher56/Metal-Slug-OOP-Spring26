#include "LevelManager.h"
#include "Level.h"

LevelManager::LevelManager() : level(nullptr), currentLevel(0) {

    this->level = new Level();
}

LevelManager::~LevelManager() {

    if (this->level != nullptr) {
        delete this->level;
        this->level = nullptr;
    }
}

void LevelManager::update(float dt) {

    if (this->level != nullptr) {

    }
}

void LevelManager::draw(RenderWindow& window, float scrollX, float scrollY) {

    if (this->level != nullptr) {
        this->level->Draw(window, scrollX, scrollY);
    }
}

Level* LevelManager::getLevel() {
    return this->level;
}

void LevelManager::nextLevel(TextureManager* tex) {

    if (this->level != nullptr) {
        delete this->level;
    }
    this->currentLevel++;
    this->level = new Level();
}

void LevelManager::saveData() {

}

void LevelManager::loadData() {

}

// ─────────────────────────────────────────────────────────────────────────────
// setLevel
// Replaces the current level with a caller-provided one.
// The caller (typically PlayState for campaign mode) creates a
// Level(NoiseProfile*) and transfers ownership here.
//
// WHY NOT DO THIS IN THE CONSTRUCTOR:
//   LevelManager is constructed before PlayState knows the game mode.
//   The default Level() (flat ground) is always safe as a starting state.
//   Campaign mode calls setLevel() after construction to swap in noise terrain.
//
// OWNERSHIP:
//   After this call, LevelManager owns newLevel and will delete it.
//   The old level is deleted here — caller must not use the old pointer.
// ─────────────────────────────────────────────────────────────────────────────
void LevelManager::setLevel(Level* newLevel) {
    // Delete the existing level (flat ground created in constructor,
    // or a previous noise level if campaign was restarted)
    if (this->level != nullptr) {
        delete this->level;
        this->level = nullptr;
    }

    // Take ownership of the new level
    this->level = newLevel;
}