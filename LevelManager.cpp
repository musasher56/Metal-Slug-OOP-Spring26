#include "LevelManager.h"
#include "Level.h"
#include "PerlinNoise.h"

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

void LevelManager::createCampaignLevel(NoiseProfile* profile) {
  
    if (this->level != nullptr) {
        delete this->level;
        this->level = nullptr;
    }

    this->level = new Level(profile);
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
