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
    
    // WHY: Load panoramic scrolling background — zoomed in (1.4x), preserve aspect ratio
    this->scroll = 0.f;
    this->bgTex.loadFromFile("resources/Sprites/background.png");
    this->bgSprite.setTexture(this->bgTex);
    // Scale to fill screen height with 1.4x zoom factor for close-up view
    float texH = static_cast<float>(this->bgTex.getSize().y);
    if (texH > 0.f) {
        this->bgScaleY = (float)SCREEN_H / texH * 1.4f;
        this->bgSprite.setScale(this->bgScaleY, this->bgScaleY);
    } else {
        this->bgScaleY = 1.f;
    }
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

        // WHY: Compute camera scroll — center on player, clamp to level bounds
        PlayerSoldier* player = this->characterManager->getCurrentCharacter();
        if (player != nullptr && lvl != nullptr) {
            float playerX = player->getPosition().x;
            float halfScreen = (float)SCREEN_W / 2.0f;
            float levelWidth = (float)(lvl->getWidth() * lvl->getCellSize());
            
            // Camera centers on player
            this->scroll = playerX - halfScreen;
            
            // Clamp: don't scroll past left edge
            if (this->scroll < 0.f) this->scroll = 0.f;
            // Clamp: don't scroll past right edge
            float maxScroll = levelWidth - (float)SCREEN_W;
            if (maxScroll < 0.f) maxScroll = 0.f;
            if (this->scroll > maxScroll) this->scroll = maxScroll;
        }
    }
    
    // WHY: Update level manager
    if (this->levelManager != nullptr) {
        this->levelManager->update(dt);
    }
}

void PlayState::render(RenderWindow& window) {
    // WHY: Draw scrolling background — offset by camera scroll, clamped on both axes
    float bgWidth = static_cast<float>(this->bgTex.getSize().x) * this->bgScaleY;
    float bgHeight = static_cast<float>(this->bgTex.getSize().y) * this->bgScaleY;
    float bgX = -this->scroll;
    float bgY = 0.f;  // TODO: When vertical scroll is added, use -scrollY here

    // Clamp horizontal: don't show empty space on left/right
    float maxBgScrollX = bgWidth - (float)SCREEN_W;
    if (maxBgScrollX < 0.f) maxBgScrollX = 0.f;
    if (bgX > 0.f) bgX = 0.f;
    if (bgX < -maxBgScrollX) bgX = -maxBgScrollX;

    // Clamp vertical: don't show empty space on top/bottom (for future vertical scroll)
    float maxBgScrollY = bgHeight - (float)SCREEN_H;
    if (maxBgScrollY < 0.f) maxBgScrollY = 0.f;
    if (bgY > 0.f) bgY = 0.f;
    if (bgY < -maxBgScrollY) bgY = -maxBgScrollY;

    this->bgSprite.setPosition(bgX, bgY);
    window.draw(this->bgSprite);

    // WHY: Draw level tiles with scroll offset
    if (this->levelManager != nullptr) {
        this->levelManager->draw(window, this->scroll);
    }
    // WHY: Draw characters with scroll offset
    if (this->characterManager != nullptr) {
        this->characterManager->draw(window, this->scroll);
    }
    // WHY: HUD draws on top without scroll (fixed to screen)
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
