#include "PlayState.h"
#include "GameStateManager.h"
#include "GameOverState.h"
#include "AudioManager.h"
// LevelSelectState removed — level select is now in MainMenu
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"
#include "BlockManager.h"
#include "DamagableEntity.h"
#include <cstdio>
#include <cmath>



PlayState::PlayState(int mode, int startChar, TextureManager* texMgr, AudioManager* audMgr, int startLvl)
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
    , showHitboxes(true), stateManager(nullptr)
    , flyingTaraPhase(0)
    , submarineSpawned(false)
    , waterBaseY(574.f)
    , startLevel(startLvl)
    , currentLevelIndex(0)
    , currentConfig(nullptr)
    , levelTransitioning(false)
    , levelTransitionTimer(0.f)
    , hudVisible(true)
{
    this->id = GSTATE_PLAY;

    this->levelManager = new LevelManager();

    this->characterManager = new CharacterManager(texMgr, audMgr, startChar);
    this->scoreManager = new ScoreManager();
    this->hud = new HUD();
    this->projectileManager = new ProjectileManager(texMgr, audMgr);
    this->enemyManager = new EnemyManager(texMgr, audMgr);
    this->enemyManager->setProjectileManager(this->projectileManager);

    this->enemyVehicleManager = new EnemyVehicleManager(texMgr, audMgr);
    this->enemyVehicleManager->setProjectileManager(this->projectileManager);

    if (this->characterManager != nullptr) {
        this->characterManager->setProjectileManager(this->projectileManager);
    }

    bool fontLoaded = this->debugFont.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
    if (!fontLoaded) fontLoaded = this->debugFont.loadFromFile("resources/font.ttf");

    this->debugText.setFont(this->debugFont);
    this->debugText.setCharacterSize(16);
    this->debugText.setFillColor(Color(0, 255, 0));
    this->debugText.setPosition(10.f, 10.f);

    this->bloodOverlayTex.loadFromFile("resources/Sprites/blood-overlay.png");
    this->bloodOverlaySprite.setTexture(this->bloodOverlayTex);
    this->bloodOverlaySprite.setScale(
        (float)SCREEN_W / (float)this->bloodOverlayTex.getSize().x * 1.1f,
        (float)SCREEN_H / (float)this->bloodOverlayTex.getSize().y * 1.1f
    );
    this->bloodOverlaySprite.setPosition(
        -(float)SCREEN_W * 0.05f,
        -(float)SCREEN_H * 0.05f
    );

    // Load the starting level (0 = campaign from beginning)
    if (this->startLevel < 0) this->startLevel = 0;
    if (this->startLevel >= TOTAL_LEVELS) this->startLevel = TOTAL_LEVELS - 1;
    this->loadLevel(this->startLevel);
}

PlayState::~PlayState() {
    if (this->enemyVehicleManager) { delete this->enemyVehicleManager; this->enemyVehicleManager = nullptr; }
    if (this->enemyManager) { delete this->enemyManager; this->enemyManager = nullptr; }
    if (this->blockManager) { delete this->blockManager;      this->blockManager = nullptr; }
    if (this->projectileManager) { delete this->projectileManager; this->projectileManager = nullptr; }
    if (this->levelManager) { delete this->levelManager;      this->levelManager = nullptr; }
    if (this->characterManager) { delete this->characterManager;  this->characterManager = nullptr; }
    if (this->scoreManager) { delete this->scoreManager;      this->scoreManager = nullptr; }
    if (this->hud) { delete this->hud;               this->hud = nullptr; }
}

// ─────────────────────────────────────────────────────────────────────────────
// loadLevel — initializes (or re-initializes) everything for a given level
// ─────────────────────────────────────────────────────────────────────────────
void PlayState::loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= TOTAL_LEVELS) return;

    this->currentLevelIndex = levelIndex;
    this->currentConfig = ALL_LEVELS[levelIndex];
    const LevelConfig* cfg = this->currentConfig;

    // ── Switch music track for the new level ──
    // Track index = levelIndex + 1 (track 0 is the title theme).
    // This handles both initial load and level-to-level transitions.
    if (this->audManager != nullptr) {
        int musicTrack = levelIndex + 1;
        if (musicTrack >= 1 && musicTrack < AudioManager::NUM_MUSIC_TRACKS) {
            this->audManager->playMusicTrack(musicTrack);
        }
    }

    // ── Reset scrolling ──
    this->scroll = 0.f;
    this->scrollY = 0.f;  // will be corrected below for flat levels

    // ── Reset vehicle/animation state ──
    this->flyingTaraPhase = 0;
    this->submarineSpawned = false;
    this->flyingTaraClock.restart();
    this->levelTransitioning = false;
    this->levelTransitionTimer = 0.f;

    // ── Clear old enemies, vehicles, projectiles ──
    if (this->enemyManager) this->enemyManager->clearAll();
    if (this->enemyVehicleManager) this->enemyVehicleManager->clearAll();
    if (this->projectileManager) this->projectileManager->clearAll();

    // ── Delete old BlockManager (mountain + ground blocks) ──
    if (this->blockManager) {
        delete this->blockManager;
        this->blockManager = nullptr;
    }

    // ── Reset the Level grid ──
    if (this->levelManager) {
        Level* lvl = this->levelManager->getLevel();
        if (lvl != nullptr) {
            // Clear all solid cells
            for (int r = 0; r < lvl->getHeight(); r++) {
                for (int c = 0; c < lvl->getWidth(); c++) {
                    lvl->setSolid(r, c, false);
                }
            }
        }
    }

    // ── Load background ──
    this->bgTex.loadFromFile(cfg->bgPath);
    this->bgSprite.setTexture(this->bgTex);
    float texH = static_cast<float>(this->bgTex.getSize().y);
    float texW = static_cast<float>(this->bgTex.getSize().x);
    if (texH > 0.f) {
        if (cfg->enableVerticalScroll) {
            // Levels with vertical scroll — scale up so BG is taller than screen
            this->bgScaleY = (float)SCREEN_H / texH * 1.4f;
        }
        else {
            // Flat levels (no vertical scroll) — scale BG to fill screen height exactly
            this->bgScaleY = (float)SCREEN_H / texH;
        }
        this->bgSprite.setScale(this->bgScaleY, this->bgScaleY);
    }
    else {
        this->bgScaleY = 1.f;
    }

    // ── Rebuild terrain ──
    if (this->levelManager != nullptr) {
        Level* lvl = this->levelManager->getLevel();
        if (lvl != nullptr) {
            this->blockManager = new BlockManager(this->texManager, this->audManager, lvl);

            this->texManager->loadTexture("dirt", "resources/Sprites/dirt.png");

            int cellSize = lvl->getCellSize();
            int groundRow = lvl->getHeight() - 1;
            int surfaceRow = groundRow - 2;
            float surfaceY = (float)(surfaceRow * cellSize);

            if (cfg->visibleGround) {
                // Normal levels — build visible dirt block ground
                this->blockManager->buildGroundTerrain(surfaceRow, 3);
            }
            else {
                // Invisible ground — mark cells solid but don't create dirt sprites
                for (int rowOff = 0; rowOff < 3; rowOff++) {
                    int row = surfaceRow + rowOff;
                    if (row >= lvl->getHeight()) break;
                    for (int col = 0; col < lvl->getWidth(); col++) {
                        lvl->setSolid(row, col, true);
                    }
                }
            }

            // Only build mountain terrain if the level config says so
            if (cfg->hasMountain) {
                this->blockManager->buildMountainTerrain(4000.f, surfaceY);
            }

            // ── Reset player position ──
            if (this->characterManager != nullptr) {
                sf::Vector2f spawnPos(200.f, surfaceY - 140.f);
                this->characterManager->initAllPositions(spawnPos);
            }

            // ── Spawn platforms from config ──
            this->spawnPlatformsFromConfig();

            // ── Spawn enemies from config ──
            this->spawnEnemiesFromConfig();

            // ── Initialize water pool shape ──
            if (cfg->hasWater) {
                this->waterShape.setPointCount(4);
                this->waterShape.setPoint(0, sf::Vector2f(cfg->waterX1, cfg->waterY1));
                this->waterShape.setPoint(1, sf::Vector2f(cfg->waterX2, cfg->waterY1));
                this->waterShape.setPoint(2, sf::Vector2f(cfg->waterX2, cfg->waterY2));
                this->waterShape.setPoint(3, sf::Vector2f(cfg->waterX1, cfg->waterY2));
                this->waterShape.setFillColor(Color(0, 30, 80, 140));  // dark water
            }
            else {
                // Empty invisible shape if no water
                this->waterShape.setPointCount(4);
                this->waterShape.setPoint(0, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(1, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(2, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(3, sf::Vector2f(0.f, 0.f));
                this->waterShape.setFillColor(Color(0, 0, 0, 0));
            }

            // ── For flat levels, set scrollY so ground lines up near bottom of screen ──
            if (!cfg->enableVerticalScroll) {
                // Place ground at ~85% down the screen (matches typical BG ground line)
                this->scrollY = surfaceY - (float)SCREEN_H * 0.85f;
                if (this->scrollY < 0.f) this->scrollY = 0.f;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// spawnEnemiesFromConfig — reads enemy list from currentConfig and spawns them
// ─────────────────────────────────────────────────────────────────────────────
void PlayState::spawnEnemiesFromConfig() {
    if (this->enemyManager == nullptr) return;
    if (this->currentConfig == nullptr) return;

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl == nullptr) return;

    int cellSize = lvl->getCellSize();
    int surfaceRow = lvl->getHeight() - 3;
    float surfaceY = (float)(surfaceRow * cellSize);
    float rebelFootOffset = 140.f;

    // Mountain coordinates for filling in x=0 enemies
    float mtBaseX = 4000.f;
    float mtTop30 = surfaceY - 11.f * 48.f;
    float mtTop65 = surfaceY - 25.f * 48.f;
    float mtTop90 = surfaceY - 25.f * 48.f;

    const LevelConfig* cfg = this->currentConfig;

    for (int i = 0; i < cfg->enemyCount && i < 40; i++) {
        const EnemySpawnEntry* e = &cfg->enemies[i];
        float ex = e->x;
        float ey = e->y;

        // Fill in Y=0 enemies with surface Y
        if (ey == 0.f) {
            ey = surfaceY - rebelFootOffset;
        }

        // Fill in mountain enemies (x=0 means mountain position)
        // Only assign mountain positions if this level has a mountain
        if (ex == 0.f) {
            if (cfg->hasMountain) {
                // Assign mountain positions based on index pattern
                if (i % 4 == 0) { ex = mtBaseX + 30.f * 48.f; ey = mtTop30 - rebelFootOffset; }
                else if (i % 4 == 1) { ex = mtBaseX + 90.f * 48.f; ey = mtTop90 - rebelFootOffset; }
                else if (i % 4 == 2) { ex = mtBaseX + 65.f * 48.f; ey = mtTop65 - rebelFootOffset; }
                else { ex = mtBaseX + 80.f * 48.f; ey = mtTop90 - rebelFootOffset; }
            }
            else {
                // No mountain — place these enemies on flat ground instead
                ex = (float)(15 + i * 8) * 48.f;
                ey = surfaceY - rebelFootOffset;
            }
        }

        switch (e->type) {
        case ENEMY_REBEL:
            this->enemyManager->spawnRebel(ex, ey);
            break;
        case ENEMY_BAZOOKA:
            this->enemyManager->spawnBazooka(ex, ey);
            break;
        case ENEMY_SHIELDED:
            this->enemyManager->spawnShielded(ex, ey);
            break;
        case ENEMY_GRENADE:
            this->enemyManager->spawnGrenade(ex, ey);
            break;
        case ENEMY_MARTIAN:
            this->enemyManager->spawnMartian(ex, ey);
            break;
        case ENEMY_PARATROOPER:
            this->enemyManager->spawnParatrooper(ex, ey - 500.f, e->landY > 0.f ? e->landY : ey);
            break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// spawnPlatformsFromConfig — reads platform list from currentConfig
// ─────────────────────────────────────────────────────────────────────────────
void PlayState::spawnPlatformsFromConfig() {
    if (this->blockManager == nullptr) return;
    if (this->currentConfig == nullptr) return;

    const LevelConfig* cfg = this->currentConfig;
    for (int i = 0; i < cfg->platformCount && i < 10; i++) {
        const PlatformSpawnEntry* p = &cfg->platforms[i];
        if (p->count > 0) {
            this->blockManager->spawnPlatform(p->startX, p->startY, p->count);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// checkLevelTransition — detects when player reaches end of current level
// ─────────────────────────────────────────────────────────────────────────────
void PlayState::checkLevelTransition() {
    PlayerSoldier* player = this->characterManager
        ? this->characterManager->getCurrentCharacter() : nullptr;
    if (player == nullptr) return;

    // Use levelWidth from config if set, otherwise fall back to grid width
    float levelWidth = 0.f;
    if (this->currentConfig != nullptr && this->currentConfig->levelWidth > 0.f) {
        levelWidth = this->currentConfig->levelWidth;
    }
    else {
        Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
        if (lvl == nullptr) return;
        levelWidth = (float)(lvl->getWidth() * lvl->getCellSize());
    }

    float playerX = player->getPosition().x;

    // Trigger transition when player is near the right edge
    if (playerX >= levelWidth - 200.f && !this->levelTransitioning) {
        this->levelTransitioning = true;
        this->levelTransitionTimer = 0.f;
    }

    if (this->levelTransitioning) {
        this->levelTransitionTimer += 1.f / 60.f;  // approx 60fps

        // Short delay before transitioning (0.5 sec)
        if (this->levelTransitionTimer >= 0.5f) {
            int nextLevel = this->currentLevelIndex + 1;

            if (nextLevel >= TOTAL_LEVELS) {
                // Beat all levels — go to GameOver (victory)
                if (this->stateManager != nullptr) {
                    int finalScore = this->scoreManager ? this->scoreManager->getScore() : 0;
                    this->stateManager->changeState(new GameOverState(finalScore));
                }
            }
            else {
                // Load next level
                this->loadLevel(nextLevel);
            }
        }
    }
}

void PlayState::update(float dt) {
    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    PlayerSoldier* player = this->characterManager ? this->characterManager->getCurrentCharacter() : nullptr;
    if (player != nullptr && player->getCurrentHP() == 0 && player->getLives() <= 0) {
        if (this->characterManager != nullptr && this->characterManager->anyCharacterAlive()) {
            this->characterManager->switchCharacter();
            player = this->characterManager->getCurrentCharacter();
        }
        else {
            if (this->stateManager != nullptr) {
                int finalScore = this->scoreManager ? this->scoreManager->getScore() : 0;
                this->stateManager->changeState(new GameOverState(finalScore));
            }
            return;
        }
    }
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

    if (player != nullptr)
        player->updateBoundingBox();

    if (this->blockManager) {
        this->blockManager->update(this->scroll, this->scrollY);
    }

    if (this->enemyManager)
        this->enemyManager->update(this->scroll, this->scrollY, lvl, player);

    if (this->enemyVehicleManager)
        this->enemyVehicleManager->update(this->scroll, this->scrollY, lvl, player, this->projectileManager);

    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->update(this->scroll, lvl);

    if (this->blockManager && this->projectileManager) {
        DamagableEntity** blocks = this->blockManager->getActiveBlocks();
        int bCount = this->blockManager->getActiveCount();
        if (bCount > 0) {
            this->projectileManager->checkEntityCollisions(blocks, bCount);
        }
    }

    if (this->enemyManager && this->projectileManager) {
        DamagableEntity** enemies = this->enemyManager->getDamagableSlots();
        int eCount = this->enemyManager->getActiveCount();
        if (eCount > 0) {
            this->projectileManager->checkPlayerBulletHits(enemies, eCount);
        }
    }

    if (this->enemyVehicleManager && this->projectileManager) {
        DamagableEntity** vehicles = this->enemyVehicleManager->getDamagableSlots();
        int vCount = this->enemyVehicleManager->getActiveCount();
        if (vCount > 0) {
            this->projectileManager->checkPlayerBulletHits(vehicles, vCount);
        }
    }

    if (this->projectileManager && player != nullptr) {
        this->projectileManager->checkEnemyBulletHitPlayer(player);
    }

    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->postEntityUpdate(this->scroll, this->scrollY, lvl);

    if (this->enemyManager)
        this->enemyManager->cleanup();

    if (this->enemyVehicleManager)
        this->enemyVehicleManager->cleanup();

    if (this->blockManager) {
        this->blockManager->cleanup();
    }

    if (player != nullptr && lvl != nullptr) {
        float playerX = player->getPosition().x;
        float levelWidth = 0.f;
        if (this->currentConfig != nullptr && this->currentConfig->levelWidth > 0.f) {
            levelWidth = this->currentConfig->levelWidth;
        }
        else {
            levelWidth = (float)(lvl->getWidth() * lvl->getCellSize());
        }
        this->scroll = playerX - (float)SCREEN_W / 2.f;
        if (this->scroll < 0.f) this->scroll = 0.f;
        float maxScroll = levelWidth - (float)SCREEN_W;
        if (maxScroll < 0.f) maxScroll = 0.f;
        if (this->scroll > maxScroll) this->scroll = maxScroll;
    }

    // Vertical scroll — only if the level config allows it
    if (player != nullptr && lvl != nullptr &&
        this->currentConfig != nullptr && this->currentConfig->enableVerticalScroll)
    {
        float playerY = player->getPosition().y;
        float levelHeight = (float)(lvl->getHeight() * lvl->getCellSize());
        this->scrollY = playerY - (float)SCREEN_H / 2.f;
        if (this->scrollY < 0.f) this->scrollY = 0.f;
        float maxScrollY = levelHeight - (float)SCREEN_H;
        if (maxScrollY < 0.f) maxScrollY = 0.f;
        if (this->scrollY > maxScrollY) this->scrollY = maxScrollY;
    }
    else if (this->currentConfig != nullptr && !this->currentConfig->enableVerticalScroll) {
        // No vertical scroll — lock camera so ground is near bottom of screen
        if (lvl != nullptr) {
            int surfaceRow = lvl->getHeight() - 3;
            float surfaceY = (float)(surfaceRow * lvl->getCellSize());
            this->scrollY = surfaceY - (float)SCREEN_H * 0.85f;
            if (this->scrollY < 0.f) this->scrollY = 0.f;
        }
    }

    // ── Check if player is in the water region ──
    if (player != nullptr && this->currentConfig != nullptr && this->currentConfig->hasWater) {
        float px = player->getPosition().x;
        float py = player->getPosition().y;
        bool inWaterNow = (px >= this->currentConfig->waterX1 &&
            px <= this->currentConfig->waterX2 &&
            py >= this->currentConfig->waterY1 &&
            py <= this->currentConfig->waterY2);
        player->setInWater(inWaterNow);
    }
    else if (player != nullptr) {
        player->setInWater(false);
    }

    if (this->levelManager)
        this->levelManager->update(dt);

    // ── FlyingTara: spawn based on config timings ──
    if (this->currentConfig != nullptr && this->enemyVehicleManager != nullptr &&
        player != nullptr && lvl != nullptr)
    {
        int passes = this->currentConfig->flyingTaraPasses;
        for (int i = 0; i < passes && i < 4; i++) {
            if (this->flyingTaraPhase == i &&
                this->flyingTaraClock.getElapsedTime().asSeconds() >= this->currentConfig->flyingTaraTimes[i])
            {
                int cellSize = lvl->getCellSize();
                int surfaceRow = lvl->getHeight() - 3;
                float surfaceY = (float)(surfaceRow * cellSize);
                float taraY = surfaceY - 500.f;

                // Alternate spawn direction: even = from left, odd = from right
                int dir = (i % 2 == 0) ? DIR_RIGHT : DIR_LEFT;
                float spawnX = (dir == DIR_RIGHT)
                    ? player->getPosition().x - (float)SCREEN_W - 200.f
                    : player->getPosition().x + (float)SCREEN_W + 200.f;

                this->enemyVehicleManager->spawnFlyingTara(spawnX, taraY, dir);
                this->flyingTaraPhase = i + 1;
            }
        }
    }

    // ── Submarine: spawn when player reaches trigger X ──
    if (this->currentConfig != nullptr && this->currentConfig->hasSubmarine &&
        !this->submarineSpawned && player != nullptr &&
        player->getPosition().x >= this->currentConfig->submarineTriggerX &&
        this->enemyVehicleManager != nullptr)
    {
        this->enemyVehicleManager->spawnSubmarine(
            this->currentConfig->submarineSpawnX,
            this->currentConfig->submarineSpawnY,
            this->currentConfig->submarineDir);
        this->submarineSpawned = true;
    }

    // ── Check level transition ──
    this->checkLevelTransition();

    // ── Update HUD data (always, even when hidden, so data is fresh on unhide) ──
    if (this->hud && this->characterManager) {
        this->hud->update(this->characterManager, this->currentLevelIndex);
    }
}

void PlayState::render(RenderWindow& window) {
    if (this->gameWindow == nullptr)
        this->gameWindow = &window;

    float bgWidth = static_cast<float>(this->bgTex.getSize().x) * this->bgScaleY;
    float bgHeight = static_cast<float>(this->bgTex.getSize().y) * this->bgScaleY;

    float bgX = -this->scroll;
    float bgY = 0.f;

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;

    if (this->currentConfig != nullptr && this->currentConfig->enableVerticalScroll) {
        // Levels with vertical scroll — align BG ground line with world ground
        const float BG_GROUND_RATIO = 0.82f;

        float groundY = 0.f;
        if (lvl != nullptr) {
            int surfaceRow = lvl->getHeight() - 3;
            groundY = (float)(surfaceRow * lvl->getCellSize());
        }

        float groundLineInSprite = bgHeight * BG_GROUND_RATIO;
        bgY = groundY - groundLineInSprite - this->scrollY;

        float maxBgY = bgHeight - (float)SCREEN_H; if (maxBgY < 0.f) maxBgY = 0.f;
        if (bgY > 0.f) bgY = 0.f;
        if (bgY < -maxBgY) bgY = -maxBgY;
    }
    else if (this->currentConfig != nullptr && !this->currentConfig->enableVerticalScroll) {
        // Flat levels — BG is scaled to fill screen height exactly
        // Keep it pinned to the screen (bgY = 0), don't scroll vertically
        bgY = 0.f;
    }

    if (bgX > 0.f) bgX = 0.f;

    // ── Draw BG (with optional tiling for levels where BG is shorter than level) ──
    if (this->currentConfig != nullptr && this->currentConfig->tileBg) {
        // Tile BG horizontally to cover the full level width
        // Calculate how many copies we need and which ones are visible
        float levelWidth = this->currentConfig->levelWidth > 0.f
            ? this->currentConfig->levelWidth
            : (float)(lvl != nullptr ? lvl->getWidth() * lvl->getCellSize() : bgWidth);

        int totalTiles = (int)(levelWidth / bgWidth) + 2;
        float viewLeft = this->scroll;
        float viewRight = this->scroll + (float)SCREEN_W;
        int firstTile = (int)(viewLeft / bgWidth);
        if (firstTile < 0) firstTile = 0;

        for (int t = firstTile; t < firstTile + totalTiles && t * bgWidth < viewRight + bgWidth; t++) {
            float tileX = (float)t * bgWidth - this->scroll;
            if (tileX + bgWidth < 0.f) continue;   // off-screen left
            if (tileX > (float)SCREEN_W) break;     // off-screen right

            this->bgSprite.setPosition(tileX, bgY);
            window.draw(this->bgSprite);
        }
    }
    else {
        // Normal single-BG draw with clamping
        float maxBgX = bgWidth - (float)SCREEN_W; if (maxBgX < 0.f) maxBgX = 0.f;
        if (bgX < -maxBgX) bgX = -maxBgX;

        this->bgSprite.setPosition(bgX, bgY);
        window.draw(this->bgSprite);
    }

    if (this->levelManager)      this->levelManager->draw(window, this->scroll, this->scrollY);
    if (this->blockManager)      this->blockManager->draw(window, this->scroll, this->scrollY);
    if (this->enemyManager)      this->enemyManager->draw(window, this->scroll, this->scrollY);
    if (this->enemyVehicleManager) this->enemyVehicleManager->draw(window, this->scroll, this->scrollY);
    if (this->characterManager)  this->characterManager->draw(window, this->scroll, this->scrollY);
    if (this->projectileManager) this->projectileManager->draw(window, this->scroll, this->scrollY);

    // ── Draw water pool (dark layer only) ──
    if (this->currentConfig != nullptr && this->currentConfig->hasWater) {
        sf::ConvexShape drawWater = this->waterShape;
        for (int i = 0; i < drawWater.getPointCount(); i++) {
            sf::Vector2f pt = drawWater.getPoint(i);
            pt.x -= this->scroll;
            pt.y -= this->scrollY;
            drawWater.setPoint(i, pt);
        }
        window.draw(drawWater);
    }

    // ── Level transition overlay ──
    if (this->levelTransitioning) {
        sf::RectangleShape overlay(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
        float alpha = this->levelTransitionTimer / 0.5f;  // 0→1 over 0.5s
        if (alpha > 1.f) alpha = 1.f;
        overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(255.f * alpha)));
        overlay.setPosition(0.f, 0.f);
        window.draw(overlay);
    }

    if (this->hudVisible && this->hud)   this->hud->draw(window);
    this->renderBloodOverlay(window);
    if (this->showHitboxes)    this->renderHitboxes(window);
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

    sprintf(line, "Level: %d/%d  Projectiles: %d\n",
        this->currentLevelIndex + 1, TOTAL_LEVELS,
        this->projectileManager ? this->projectileManager->getActiveCount() : -1);
    append(line);
    sprintf(line, "Blocks: %d / %d  Enemies: %d  Vehicles: %d\n",
        this->blockManager ? this->blockManager->getActiveCount() : -1,
        this->blockManager ? this->blockManager->getTotalCount() : -1,
        this->enemyManager ? this->enemyManager->getActiveCount() : -1,
        this->enemyVehicleManager ? this->enemyVehicleManager->getActiveCount() : -1);
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
    if (player) {
        sprintf(line, "Weapon: %s  |  ", player->getCurrentWeaponName());
        append(line);
    }
    sprintf(line, "X=Shoot  V=Melee  C=Grenade  Arrows=Move  Space=Jump  Z=Switch  Q=CycleWeapon  H=Hitboxes  T=ToggleHUD");
    append(line);

    this->debugText.setString(buf);
    RectangleShape bg(sf::Vector2f(620.f, 130.f));
    bg.setFillColor(Color(0, 0, 0, 170));
    bg.setPosition(5.f, 5.f);
    window.draw(bg);
    window.draw(this->debugText);
}

void PlayState::renderHitboxes(RenderWindow& window) {
    PlayerSoldier* player = this->characterManager
        ? this->characterManager->getCurrentCharacter() : nullptr;

    if (player != nullptr) {
        IntRect box = player->getBoundingBox();
        RectangleShape rect(sf::Vector2f((float)box.width, (float)box.height));
        rect.setPosition((float)box.left - this->scroll,
            (float)box.top - this->scrollY);
        rect.setFillColor(Color(0, 255, 0, 50));
        rect.setOutlineColor(Color(0, 255, 0));
        rect.setOutlineThickness(2.f);
        window.draw(rect);
    }

    if (this->enemyManager != nullptr) {
        DamagableEntity** enemies = this->enemyManager->getDamagableSlots();
        int count = this->enemyManager->getActiveCount();
        for (int i = 0; i < count; i++) {
            DamagableEntity* e = enemies[i];
            if (e == nullptr) continue;
            IntRect box = e->getBoundingBox();
            RectangleShape rect(sf::Vector2f((float)box.width, (float)box.height));
            rect.setPosition((float)box.left - this->scroll,
                (float)box.top - this->scrollY);
            rect.setFillColor(Color(255, 50, 50, 50));
            rect.setOutlineColor(Color(255, 50, 50));
            rect.setOutlineThickness(2.f);
            window.draw(rect);
        }
    }

    if (this->enemyVehicleManager != nullptr) {
        DamagableEntity** vehicles = this->enemyVehicleManager->getDamagableSlots();
        int vCount = this->enemyVehicleManager->getActiveCount();
        for (int i = 0; i < vCount; i++) {
            DamagableEntity* v = vehicles[i];
            if (v == nullptr) continue;
            IntRect box = v->getBoundingBox();
            RectangleShape rect(sf::Vector2f((float)box.width, (float)box.height));
            rect.setPosition((float)box.left - this->scroll,
                (float)box.top - this->scrollY);
            rect.setFillColor(Color(255, 100, 255, 50));
            rect.setOutlineColor(Color(255, 100, 255));
            rect.setOutlineThickness(2.f);
            window.draw(rect);
        }
    }

    if (this->projectileManager != nullptr) {
        Projectile** projs = this->projectileManager->getSlots();
        int pCount = this->projectileManager->getActiveCount();
        for (int i = 0; i < pCount; i++) {
            Projectile* p = projs[i];
            if (p == nullptr || !p->getStatus()) continue;

            IntRect box = p->getBoundingBox();
            RectangleShape rect(sf::Vector2f((float)box.width, (float)box.height));
            rect.setPosition((float)box.left - this->scroll,
                (float)box.top - this->scrollY);
            rect.setFillColor(Color(255, 255, 0, 80));
            rect.setOutlineColor(Color(255, 255, 0));
            rect.setOutlineThickness(1.f);
            window.draw(rect);
        }
    }
}

void PlayState::renderBloodOverlay(RenderWindow& window) {
    PlayerSoldier* player = this->characterManager
        ? this->characterManager->getCurrentCharacter() : nullptr;
    if (player == nullptr) return;
    if (player->getCurrentHP() > 0 && player->getCurrentHP() <= 1) {
        window.draw(this->bloodOverlaySprite);
    }
}

void PlayState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::H) {
        this->showHitboxes = !this->showHitboxes;
    }

    // T key toggles ALL on-screen overlays: HUD (score/hearts/weapon) AND
    // the debug panel.  Previously only the HUD portion was hidden, leaving
    // the debug text visible — players couldn't tell the toggle was working.
    // Now T hides/shows everything at once for a clean screenshot mode.
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::T) {
        this->hudVisible = !this->hudVisible;
        this->debugMode  = !this->hudVisible;   // hide debug when HUD is hidden
        this->showHitboxes = !this->hudVisible; // hide hitboxes too
    }

    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Z) {
        if (this->characterManager != nullptr) {
            this->characterManager->switchCharacter();
        }
    }

    if (this->characterManager)
        this->characterManager->handleInput(event);
}

void PlayState::onEnter() {
    // Start level music when entering PlayState.
    // Track index = currentLevelIndex + 1 (track 0 is the title theme).
    if (this->audManager != nullptr) {
        int musicTrack = this->currentLevelIndex + 1;
        if (musicTrack >= 1 && musicTrack < AudioManager::NUM_MUSIC_TRACKS) {
            this->audManager->playMusicTrack(musicTrack);
        }
    }
}

void PlayState::onExit() {
    // Stop music when leaving PlayState (game over, returning to menu, etc.).
    if (this->audManager != nullptr) {
        this->audManager->stopMusic();
    }
}

// ── Legacy spawn functions (kept for compatibility, not used by loadLevel) ──

void PlayState::spawnTestBlocks() {
    if (this->blockManager == nullptr) return;

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl == nullptr) return;

    this->blockManager->spawnPlatform(
        static_cast<float>(8 * 48),
        static_cast<float>(32 * 48),
        8
    );

    this->blockManager->spawnPlatform(
        static_cast<float>(22 * 48),
        static_cast<float>(30 * 48),
        7
    );
    this->blockManager->spawnPlatform(
        static_cast<float>(32 * 48),
        static_cast<float>(30 * 48),
        4
    );

    this->blockManager->spawnPlatform(
        static_cast<float>(42 * 48),
        static_cast<float>(30 * 48),
        10
    );
}

void PlayState::spawnTestEnemies() {
    if (this->enemyManager == nullptr) return;

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl == nullptr) return;

    int cellSize = lvl->getCellSize();
    int surfaceRow = lvl->getHeight() - 3;
    float surfaceY = (float)(surfaceRow * cellSize);

    float rebelFootOffset = 140.f;

    this->enemyManager->spawnMartian(15.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnRebel(35.f * 48.f, surfaceY - rebelFootOffset);

    float platY1 = 32.f * 48.f;
    float platY2 = 30.f * 48.f;

    this->enemyManager->spawnRebel(10.f * 48.f, platY1 - rebelFootOffset);
    this->enemyManager->spawnRebel(24.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnRebel(34.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnRebel(48.f * 48.f, platY2 - rebelFootOffset);

    float mtBaseX = 4000.f;
    float mtTop30 = surfaceY - 11.f * 48.f;
    float mtTop90 = surfaceY - 25.f * 48.f;
    this->enemyManager->spawnRebel(mtBaseX + 30.f * 48.f, mtTop30 - rebelFootOffset);
    this->enemyManager->spawnRebel(mtBaseX + 90.f * 48.f, mtTop90 - rebelFootOffset);

    this->enemyManager->spawnBazooka(25.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnBazooka(50.f * 48.f, surfaceY - rebelFootOffset);

    this->enemyManager->spawnShielded(20.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnShielded(40.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnShielded(mtBaseX + 90.f * 48.f, mtTop90 - rebelFootOffset);

    this->enemyManager->spawnGrenade(12.f * 48.f, platY1 - rebelFootOffset);
    this->enemyManager->spawnGrenade(33.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnGrenade(mtBaseX + 30.f * 48.f, mtTop30 - rebelFootOffset);

    float mtPeak = surfaceY - 25.f * 48.f;
    this->enemyManager->spawnMartian(mtBaseX + 65.f * 48.f, mtPeak - rebelFootOffset);

    float paraLandingY = mtPeak - rebelFootOffset;
    this->enemyManager->spawnParatrooper(
        mtBaseX + 80.f * 48.f,
        paraLandingY - 500.f,
        paraLandingY
    );
}