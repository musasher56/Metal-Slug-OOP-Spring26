#include "PlayState.h"
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"
#include "BlockManager.h"
#include "DamagableEntity.h"
#include <cstdio>

PlayState::PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr)
    : entityManager(nullptr), enemyManager(nullptr), enemyVehicleManager(nullptr),
    projectileManager(nullptr), collectibleManager(nullptr),
    texManager(texMgr), audManager(audMgr),
    level(nullptr), characterManager(nullptr), levelManager(nullptr),
    scoreManager(nullptr), hud(nullptr), blockManager(nullptr),
    gameMode(mode),
    movingLeft(false), movingRight(false),
    gameWindow(nullptr),
    lastMouseWorld(0.f, 0.f),
    debugMode(true)
{
    this->id = GSTATE_PLAY;

    this->levelManager = new LevelManager();
    this->characterManager = new CharacterManager(texMgr, audMgr);
    this->scoreManager = new ScoreManager();
    this->hud = new HUD();
    this->projectileManager = new ProjectileManager(texMgr, audMgr);

    PlayerSoldier* player = this->characterManager->getCurrentCharacter();
    if (player != nullptr)
        player->setProjectileManager(this->projectileManager);

    bool fontLoaded = this->debugFont.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("resources/font.ttf");

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
    }
    else {
        this->bgScaleY = 1.f;
    }

    if (this->levelManager != nullptr) {
        Level* lvl = this->levelManager->getLevel();
        if (lvl != nullptr) {
            this->blockManager = new BlockManager(texMgr, audMgr, lvl);
            this->spawnTestBlocks();
        }
    }
}

PlayState::~PlayState() {
    if (this->blockManager) { delete this->blockManager;      this->blockManager = nullptr; }
    if (this->projectileManager) { delete this->projectileManager; this->projectileManager = nullptr; }
    if (this->levelManager) { delete this->levelManager;      this->levelManager = nullptr; }
    if (this->characterManager) { delete this->characterManager;  this->characterManager = nullptr; }
    if (this->scoreManager) { delete this->scoreManager;      this->scoreManager = nullptr; }
    if (this->hud) { delete this->hud;               this->hud = nullptr; }
}

void PlayState::update(float dt) {
    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    PlayerSoldier* player = this->characterManager ? this->characterManager->getCurrentCharacter() : nullptr;

    if (player != nullptr && this->gameWindow != nullptr) {
        sf::Vector2i mp = Mouse::getPosition(*this->gameWindow);
        sf::Vector2f mouseWorld(
            static_cast<float>(mp.x) + this->scroll,
            static_cast<float>(mp.y)
        );
        this->lastMouseWorld = mouseWorld;
        player->updateAim(mouseWorld);
    }

    // 1. Update player movement + gravity + handleCollision (checks grid — blocks are solid)
    if (this->characterManager)
        this->characterManager->update(dt, lvl);

    if (player != nullptr)
        player->handleInput();

    // 2. Update block destruction animations
    if (this->blockManager) {
        this->blockManager->update(this->scroll);
    }

    // 3. PHASE 1: Move projectiles (no tile collision yet)
    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->update(this->scroll, lvl);

    // 4. Bullet vs block entity collision (after move, before tile check)
    //    Damaged blocks clear their grid cells here.
    if (this->blockManager && this->projectileManager) {
        DamagableEntity** blocks = this->blockManager->getActiveBlocks();
        int bCount = this->blockManager->getActiveCount();
        if (bCount > 0) {
            this->projectileManager->checkEntityCollisions(blocks, bCount);
        }
    }

    // 5. PHASE 3: Tile collision + bounds + cleanup for projectiles
    //    Blocks that took damage already cleared their grid cells,
    //    so bullets pass through destroyed block positions.
    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->postEntityUpdate(this->scroll, lvl);

    // 6. Clean up finished block destruction animations
    if (this->blockManager) {
        this->blockManager->cleanup();
    }

    if (player != nullptr && lvl != nullptr) {
        float playerX = player->getPosition().x;
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

    float bgWidth = static_cast<float>(this->bgTex.getSize().x) * this->bgScaleY;
    float bgHeight = static_cast<float>(this->bgTex.getSize().y) * this->bgScaleY;
    float bgX = -this->scroll;
    float bgY = -(bgHeight - (float)SCREEN_H);

    if (bgX > 0.f) bgX = 0.f;
    float maxBgX = bgWidth - (float)SCREEN_W; if (maxBgX < 0.f) maxBgX = 0.f;
    float maxBgY = bgHeight - (float)SCREEN_H; if (maxBgY < 0.f) maxBgY = 0.f;
    if (bgX < -maxBgX) bgX = -maxBgX;
    if (bgY > 0.f) bgY = 0.f;
    if (bgY < -maxBgY) bgY = -maxBgY;

    this->bgSprite.setPosition(bgX, bgY);
    window.draw(this->bgSprite);

    if (this->levelManager)      this->levelManager->draw(window, this->scroll);
    if (this->blockManager)      this->blockManager->draw(window, this->scroll);
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
    sprintf(line, "Blocks: %d / %d\n",
        this->blockManager ? this->blockManager->getActiveCount() : -1,
        this->blockManager ? this->blockManager->getTotalCount() : -1);
    append(line);
    sprintf(line, "Aim: %.1f deg  |  Mouse: (%.0f, %.0f)\n",
        player ? player->getAimAngle() : -1.f,
        this->lastMouseWorld.x, this->lastMouseWorld.y);
    append(line);
    if (player) {
        sf::Vector2f p = player->getPosition();
        sprintf(line, "Player: (%.0f, %.0f)  Scroll: %.0f\n", p.x, p.y, this->scroll);
    }
    else {
        sprintf(line, "Player: NULL\n");
    }
    append(line);
    sprintf(line, "Lag: %s\n", this->gameWindow ? "ZERO" : "1-frame");
    append(line);
    sprintf(line, "Z=Shoot  X=Grenade  Arrows=Move  Space=Jump");
    append(line);

    this->debugText.setString(buf);
    RectangleShape bg(sf::Vector2f(380.f, 110.f));
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
void PlayState::onExit() {}

void PlayState::spawnTestBlocks() {
    if (this->blockManager == nullptr) return;

    // Platform 1: 8 blocks at row 12 (just above ground at row 14)
    this->blockManager->spawnPlatform(
        static_cast<float>(8 * 48),
        static_cast<float>(12 * 48),
        8
    );

    // Platform 2: 6 blocks at row 10 (floating higher)
    this->blockManager->spawnPlatform(
        static_cast<float>(22 * 48),
        static_cast<float>(10 * 48),
        6
    );

    // Platform 3: 4 blocks at row 11
    this->blockManager->spawnPlatform(
        static_cast<float>(40 * 48),
        static_cast<float>(11 * 48),
        4
    );
}