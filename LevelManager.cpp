#include "LevelManager.h"
#include "Level.h"

LevelManager::LevelManager() : level(nullptr), currentLevel(0) {
    // WHY: Create initial level
    this->level = new Level();
}

LevelManager::~LevelManager() {
    // WHY: Delete owned level
    if (this->level != nullptr) {
        delete this->level;
        this->level = nullptr;
    }
}

void LevelManager::update(float dt) {
    // WHY: Update current level (rain, etc.)
    if (this->level != nullptr) {
        // Level update handled in PlayState
    }
}

void LevelManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    // WHY: Draw current level with both scroll offsets for vertical scrolling
    if (this->level != nullptr) {
        this->level->Draw(window, scrollX, scrollY);
    }
}

Level* LevelManager::getLevel() {
    return this->level;
}

void LevelManager::nextLevel(TextureManager* tex) {
    // WHY: Load next level
    if (this->level != nullptr) {
        delete this->level;
    }
    this->currentLevel++;
    this->level = new Level();
}

void LevelManager::saveData() {
    // TODO: Implement save
}

void LevelManager::loadData() {
    // TODO: Implement load
}
