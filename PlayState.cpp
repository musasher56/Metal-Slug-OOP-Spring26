#include "PlayState.h"
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"

PlayState::PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr) 
    : entityManager(nullptr), enemyManager(nullptr), enemyVehicleManager(nullptr),
      projectileManager(nullptr), collectibleManager(nullptr),
      texManager(texMgr), audManager(audMgr),
      level(nullptr), characterManager(nullptr), levelManager(nullptr),
      scoreManager(nullptr), hud(nullptr), gameMode(mode),
      movingLeft(false), movingRight(false) {
    this->id = GSTATE_PLAY;
    
    // WHY: Initialize level manager and character manager for basic gameplay
    this->levelManager = new LevelManager();
    this->characterManager = new CharacterManager(texMgr, audMgr);
    this->scoreManager = new ScoreManager();
    this->hud = new HUD();
    
    // WHY: Load background texture for gameplay
    this->bgTex.loadFromFile("resources/Sprites/tempbg.jpeg");
    this->bgSprite.setTexture(this->bgTex);
    float sx = (float)SCREEN_W / (float)this->bgTex.getSize().x;
    float sy = (float)SCREEN_H / (float)this->bgTex.getSize().y;
    this->bgSprite.setScale(sx, sy);
}

PlayState::~PlayState() {
    // WHY: PlayState owns managers, must delete them
    if (this->levelManager != nullptr) {
        delete this->levelManager;
        this->levelManager = nullptr;
    }
    if (this->characterManager != nullptr) {
        delete this->characterManager;
        this->characterManager = nullptr;
    }
    if (this->scoreManager != nullptr) {
        delete this->scoreManager;
        this->scoreManager = nullptr;
    }
    if (this->hud != nullptr) {
        delete this->hud;
        this->hud = nullptr;
    }
    // WHY: texManager and audManager are owned by Game, just references here
}

void PlayState::update(float dt) {
    // WHY: Update character manager (handles all PlayerSoldiers)
    if (this->characterManager != nullptr && this->levelManager != nullptr) {
        Level* lvl = this->levelManager->getLevel();
        this->characterManager->update(dt, lvl);
    }
    
    // WHY: Update level manager
    if (this->levelManager != nullptr) {
        this->levelManager->update(dt);
    }
}

void PlayState::render(RenderWindow& window) {
    // WHY: Render background, level, and all characters
    window.draw(this->bgSprite);
    if (this->levelManager != nullptr) {
        this->levelManager->draw(window);
    }
    if (this->characterManager != nullptr) {
        this->characterManager->draw(window);
    }
    if (this->hud != nullptr) {
        this->hud->draw(window);
    }
}

void PlayState::handleEvent(Event& event) {
    // WHY: Delegate input handling to character manager
    if (this->characterManager != nullptr) {
        this->characterManager->handleInput(event);
    }
}

void PlayState::onEnter() {
    // WHY: Request focus when entering play state
}

void PlayState::onExit() {
    // WHY: Cleanup when exiting play state
}
