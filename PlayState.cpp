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
    scroll(0.f), scrollY(0.f),
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
    this->scrollY = 0.f;
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

            this->texManager->loadTexture("dirt", "resources/Sprites/dirt.png");

            int cellSize = lvl->getCellSize();
            int groundRow = lvl->getHeight() - 1;         // row 39
            int surfaceRow = groundRow - 2;               // row 37 (new walking surface)
            float surfaceY = (float)(surfaceRow * cellSize); // 1776

            // FIX: Fill 3 rows of visible dirt blocks for the ground floor.
            // Row 37 = walking surface, rows 38-39 = underground depth.
            // 240 cols × 3 rows = 720 blocks.
            this->blockManager->buildGroundTerrain(surfaceRow, 3);

            // Mountain starts from the new ground surface
            this->blockManager->buildMountainTerrain(4000.f, surfaceY);

            if (player != nullptr) {
                // FIX: Spawn on new surface (row 37 top = 1776, minus hitbox height 140)
                player->position = sf::Vector2f(200.f, surfaceY - 140.f);
                player->updateBoundingBox();
            }

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
            static_cast<float>(mp.y) + this->scrollY
        );
        this->lastMouseWorld = mouseWorld;
        player->updateAim(mouseWorld);
    }

    if (this->characterManager)
        this->characterManager->update(dt, lvl);

    if (player != nullptr)
        player->handleInput();

    if (this->blockManager) {
        this->blockManager->update(this->scroll, this->scrollY);
    }

    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->update(this->scroll, lvl);

    if (this->blockManager && this->projectileManager) {
        DamagableEntity** blocks = this->blockManager->getActiveBlocks();
        int bCount = this->blockManager->getActiveCount();
        if (bCount > 0) {
            this->projectileManager->checkEntityCollisions(blocks, bCount);
        }
    }

    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->postEntityUpdate(this->scroll, this->scrollY, lvl);

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

    if (player != nullptr && lvl != nullptr) {
        float playerY = player->getPosition().y;
        float levelHeight = (float)(lvl->getHeight() * lvl->getCellSize());
        this->scrollY = playerY - (float)SCREEN_H / 2.f;
        if (this->scrollY < 0.f) this->scrollY = 0.f;
        float maxScrollY = levelHeight - (float)SCREEN_H;
        if (maxScrollY < 0.f) maxScrollY = 0.f;
        if (this->scrollY > maxScrollY) this->scrollY = maxScrollY;
    }

    if (this->levelManager)
        this->levelManager->update(dt);
}

void PlayState::render(RenderWindow& window) {
    if (this->gameWindow == nullptr)
        this->gameWindow = &window;

    float bgWidth = static_cast<float>(this->bgTex.getSize().x) * this->bgScaleY;
    float bgHeight = static_cast<float>(this->bgTex.getSize().y) * this->bgScaleY;

    const float BG_GROUND_RATIO = 0.82f;

    float groundY = 0.f;
    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl != nullptr) {
        int surfaceRow = lvl->getHeight() - 3; // row 37
        groundY = (float)(surfaceRow * lvl->getCellSize());
    }

    float groundLineInSprite = bgHeight * BG_GROUND_RATIO;
    float bgX = -this->scroll;
    float bgY = groundY - groundLineInSprite - this->scrollY;

    if (bgX > 0.f) bgX = 0.f;
    float maxBgX = bgWidth - (float)SCREEN_W; if (maxBgX < 0.f) maxBgX = 0.f;
    if (bgX < -maxBgX) bgX = -maxBgX;
    float maxBgY = bgHeight - (float)SCREEN_H; if (maxBgY < 0.f) maxBgY = 0.f;
    if (bgY > 0.f) bgY = 0.f;
    if (bgY < -maxBgY) bgY = -maxBgY;

    this->bgSprite.setPosition(bgX, bgY);
    window.draw(this->bgSprite);

    if (this->levelManager)      this->levelManager->draw(window, this->scroll, this->scrollY);
    if (this->blockManager)      this->blockManager->draw(window, this->scroll, this->scrollY);
    if (this->characterManager)  this->characterManager->draw(window, this->scroll, this->scrollY);
    if (this->projectileManager) this->projectileManager->draw(window, this->scroll, this->scrollY);
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
    sprintf(line, "Blocks: %d / %d  Mountain: %d\n",
        this->blockManager ? this->blockManager->getActiveCount() : -1,
        this->blockManager ? this->blockManager->getTotalCount() : -1,
        this->blockManager ? 0 : 0);
    append(line);
    sprintf(line, "Aim: %.1f deg  |  Mouse: (%.0f, %.0f)\n",
        player ? player->getAimAngle() : -1.f,
        this->lastMouseWorld.x, this->lastMouseWorld.y);
    append(line);
    if (player) {
        sf::Vector2f p = player->getPosition();
        sprintf(line, "Player: (%.0f, %.0f)  Scroll: (%.0f, %.0f)\n",
            p.x, p.y, this->scroll, this->scrollY);
    }
    else {
        sprintf(line, "Player: NULL\n");
    }
    append(line);
    sprintf(line, "Z=Shoot  X=Grenade  Arrows=Move  Space=Jump");
    append(line);

    this->debugText.setString(buf);
    RectangleShape bg(sf::Vector2f(400.f, 110.f));
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

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl == nullptr) return;

    // FIX: Platforms lowered from rows 27-30 to rows 33-34.
    // With jump velocity -20, max jump height ≈ 250px.
    // Player stands at y=1636, bottom at y=1776 (surface row 37).
    // Player bottom at jump apex ≈ 1776 - 250 = 1526.
    // Row 34 (y=1632): easy reach (bottom 1526 < 1632)
    // Row 33 (y=1584): moderate reach (bottom 1526 < 1584, 58px margin)

    this->blockManager->spawnPlatform(
        static_cast<float>(8 * 48),
        static_cast<float>(34 * 48),   // was 30
        8
    );

    this->blockManager->spawnPlatform(
        static_cast<float>(22 * 48),
        static_cast<float>(33 * 48),   // was 28
        7
    );
    this->blockManager->spawnPlatform(
        static_cast<float>(32 * 48),
        static_cast<float>(33 * 48),   // was 29
        4
    );

    this->blockManager->spawnPlatform(
        static_cast<float>(42 * 48),
        static_cast<float>(34 * 48),   // was 27
        10
    );
}