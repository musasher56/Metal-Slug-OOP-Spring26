#include "PlayState.h"
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"
#include <cstdio>

PlayState::PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr)
    : entityManager(nullptr), enemyManager(nullptr), enemyVehicleManager(nullptr),
      projectileManager(nullptr), collectibleManager(nullptr),
      texManager(texMgr), audManager(audMgr),
      level(nullptr), characterManager(nullptr), levelManager(nullptr),
      scoreManager(nullptr), hud(nullptr), gameMode(mode),
      movingLeft(false), movingRight(false),
      gameWindow(nullptr),
      lastMouseWorld(0.f, 0.f),
      debugMode(true)
{
    this->id = GSTATE_PLAY;

    this->levelManager      = new LevelManager();
    this->characterManager  = new CharacterManager(texMgr, audMgr);
    this->scoreManager      = new ScoreManager();
    this->hud               = new HUD();

    // ProjectileManager: class name uppercase M, new uses class name
    this->projectileManager = new ProjectileManager(texMgr, audMgr);

    PlayerSoldier* player = this->characterManager->getCurrentCharacter();
    if (player != nullptr)
        player->setProjectileManager(this->projectileManager);

    bool fontLoaded = this->debugFont.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) this->debugFont.loadFromFile("resources/font.ttf");

    this->debugText.setFont(this->debugFont);
    this->debugText.setCharacterSize(16);
    this->debugText.setFillColor(Color(0, 255, 0));
    this->debugText.setPosition(10.f, 10.f);

    this->scroll = 0.f;
    this->bgTex.loadFromFile("resources/Sprites/background.png");
    this->bgSprite.setTexture(this->bgTex);
    float texH = static_cast<float>(this->bgTex.getSize().y);
    if (texH > 0.f) {
        this->bgScaleY = (float)SCREEN_H / texH * 1.4f;
        this->bgSprite.setScale(this->bgScaleY, this->bgScaleY);
    } else {
        this->bgScaleY = 1.f;
    }
}

PlayState::~PlayState() {
    if (this->projectileManager) { delete this->projectileManager; this->projectileManager = nullptr; }
    if (this->levelManager)      { delete this->levelManager;      this->levelManager      = nullptr; }
    if (this->characterManager)  { delete this->characterManager;  this->characterManager  = nullptr; }
    if (this->scoreManager)      { delete this->scoreManager;      this->scoreManager      = nullptr; }
    if (this->hud)               { delete this->hud;               this->hud               = nullptr; }
}

void PlayState::update(float dt) {
    Level*         lvl    = this->levelManager    ? this->levelManager->getLevel()                : nullptr;
    PlayerSoldier* player = this->characterManager? this->characterManager->getCurrentCharacter() : nullptr;

    // Zero-lag aim: read mouse this frame using stored window pointer
    if (player != nullptr && this->gameWindow != nullptr) {
        sf::Vector2i mp = Mouse::getPosition(*this->gameWindow);
        sf::Vector2f mouseWorld(
            static_cast<float>(mp.x) + this->scroll,
            static_cast<float>(mp.y)
        );
        this->lastMouseWorld = mouseWorld;
        player->updateAim(mouseWorld);
    }

    if (this->characterManager)
        this->characterManager->update(dt, lvl);

    if (player != nullptr)
        player->handleInput();

    if (this->projectileManager)
        this->projectileManager->update(this->scroll, lvl);

    if (player != nullptr && lvl != nullptr) {
        float playerX    = player->getPosition().x;
        float levelWidth = (float)(lvl->getWidth() * lvl->getCellSize());
        this->scroll = playerX - (float)SCREEN_W / 2.f;
        if (this->scroll < 0.f) this->scroll = 0.f;
        float maxScroll = levelWidth - (float)SCREEN_W;
        if (maxScroll < 0.f) maxScroll = 0.f;
        if (this->scroll > maxScroll) this->scroll = maxScroll;
    }

    if (this->levelManager)
        this->levelManager->update(dt);
}

void PlayState::render(RenderWindow& window) {
    if (this->gameWindow == nullptr)
        this->gameWindow = &window;

    float bgWidth  = static_cast<float>(this->bgTex.getSize().x) * this->bgScaleY;
    float bgHeight = static_cast<float>(this->bgTex.getSize().y) * this->bgScaleY;
    float bgX = -this->scroll;
    float bgY = -(bgHeight - (float)SCREEN_H);

    if (bgX > 0.f) bgX = 0.f;
    float maxBgX = bgWidth  - (float)SCREEN_W; if (maxBgX < 0.f) maxBgX = 0.f;
    float maxBgY = bgHeight - (float)SCREEN_H; if (maxBgY < 0.f) maxBgY = 0.f;
    if (bgX < -maxBgX) bgX = -maxBgX;
    if (bgY > 0.f) bgY = 0.f;
    if (bgY < -maxBgY) bgY = -maxBgY;

    this->bgSprite.setPosition(bgX, bgY);
    window.draw(this->bgSprite);

    if (this->levelManager)      this->levelManager->draw(window, this->scroll);
    if (this->characterManager)  this->characterManager->draw(window, this->scroll);
    if (this->projectileManager) this->projectileManager->draw(window, this->scroll);
    if (this->hud)               this->hud->draw(window);
    if (this->debugMode)         this->renderDebug(window);
}

void PlayState::renderDebug(RenderWindow& window) {
    PlayerSoldier* player = this->characterManager
                          ? this->characterManager->getCurrentCharacter() : nullptr;
    char buf[512]; buf[0] = '\0';
    char line[128];
    int k = 0;
    auto append = [&](const char* src) {
        int j = 0; while (src[j]) buf[k++] = src[j++]; buf[k] = '\0';
    };

    sprintf(line, "Projectiles: %d\n",
            this->projectileManager ? this->projectileManager->getActiveCount() : -1);
    append(line);
    sprintf(line, "Aim: %.1f deg  |  Mouse: (%.0f, %.0f)\n",
            player ? player->getAimAngle() : -1.f,
            this->lastMouseWorld.x, this->lastMouseWorld.y);
    append(line);
    if (player) {
        sf::Vector2f p = player->getPosition();
        sprintf(line, "Player: (%.0f, %.0f)  Scroll: %.0f\n", p.x, p.y, this->scroll);
    } else {
        sprintf(line, "Player: NULL\n");
    }
    append(line);
    sprintf(line, "Lag: %s\n", this->gameWindow ? "ZERO" : "1-frame");
    append(line);
    sprintf(line, "Z=Shoot  X=Grenade  Arrows=Move  Space=Jump");
    append(line);

    this->debugText.setString(buf);
    RectangleShape bg(sf::Vector2f(380.f, 95.f));
    bg.setFillColor(Color(0, 0, 0, 170));
    bg.setPosition(5.f, 5.f);
    window.draw(bg);
    window.draw(this->debugText);
}

void PlayState::handleEvent(Event& event) {
    if (this->characterManager)
        this->characterManager->handleInput(event);
}

void PlayState::onEnter() {}
void PlayState::onExit()  {}