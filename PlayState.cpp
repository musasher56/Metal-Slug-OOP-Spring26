#include "PlayState.h"

PlayState::PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr) 
    : entityManager(nullptr), levelManager(nullptr), characterManager(nullptr),
      enemyManager(nullptr), enemyVehicleManager(nullptr), projectileManager(nullptr),
      collectibleManager(nullptr), scoreManager(nullptr), hud(nullptr), 
      texManager(texMgr), audManager(audMgr),
      level(nullptr), player(nullptr), gameMode(mode),
      movingLeft(false), movingRight(false) {
    this->id = STATE_PLAY;
    
    // WHY: Initialize level and player for basic gameplay (temporary until managers are implemented)
    this->level = new Level();
    this->player = new Player();
    
    // WHY: Load background texture for gameplay
    this->bgTex.loadFromFile("resources/Sprites/tempbg.jpeg");
    this->bgSprite.setTexture(this->bgTex);
    float sx = (float)SCREEN_W / (float)this->bgTex.getSize().x;
    float sy = (float)SCREEN_H / (float)this->bgTex.getSize().y;
    this->bgSprite.setScale(sx, sy);
}

PlayState::~PlayState() {
    // WHY: PlayState owns level and player, must delete them
    if (this->level != nullptr) {
        delete this->level;
        this->level = nullptr;
    }
    if (this->player != nullptr) {
        delete this->player;
        this->player = nullptr;
    }
    // WHY: texManager and audManager are owned by Game, just references here
}

void PlayState::update(float dt) {
    // WHY: Apply movement every frame using real-time input
    if (this->player != nullptr) {
        bool left  = Keyboard::isKeyPressed(Keyboard::Left);
        bool right = Keyboard::isKeyPressed(Keyboard::Right);

        if (right && !left) {
            this->player->moveRight();
        } else if (left && !right) {
            this->player->moveLeft();
        } else {
            this->player->stop();
        }
    }

    // WHY: Update player physics and collision with level
    if (this->player != nullptr && this->level != nullptr) {
        this->player->update(
            this->level->getLvl(),
            this->level->getHeight(),
            this->level->getWidth(),
            this->level->getCellSize()
        );
    }
}

void PlayState::render(RenderWindow& window) {
    // WHY: Render background, level, and player sprite
    window.draw(this->bgSprite);
    if (this->level != nullptr) {
        this->level->Draw(window);
    }
    if (this->player != nullptr) {
        this->player->draw(window);
    }
}

void PlayState::handleEvent(Event& event) {
    // WHY: Only handle jump on key press
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Space && this->player != nullptr) {
            this->player->jump();
        }
    }
}

void PlayState::onEnter() {
    // WHY: Request focus when entering play state
}

void PlayState::onExit() {
    // WHY: Cleanup when exiting play state
}
