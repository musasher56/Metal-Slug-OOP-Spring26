#include "PlayState.h"
#include "GameStateManager.h"
#include "GameOverState.h"
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"
#include "BlockManager.h"
#include "DamagableEntity.h"
#include "PerlinNoise.h"
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
    stateManager(nullptr)
    , flyingTaraPhase(0)
    , submarineSpawned(false)
    , waterBaseY(574.f)
    , startLevel(startLvl)
    , currentLevelIndex(0)
    , currentConfig(nullptr)
    , levelTransitioning(false)
    , levelTransitionTimer(0.f)
    , bossFelledTriggered(false)
    , bossesSpawned(0)
    , bossesDefeated(0)
    , fractalNoise(nullptr)
    , campaignProfile(nullptr)
    , campaignSeed(42)
    , campaignProfileType(NOISE_NORMAL)
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

    // Load the starting level
    // Campaign mode uses procedural terrain; Survival uses predefined levels
    if (this->gameMode == MODE_CAMPAIGN) {
        this->loadCampaignLevel();
    }
    else {
        if (this->startLevel < 0) this->startLevel = 0;
        if (this->startLevel >= TOTAL_LEVELS) this->startLevel = TOTAL_LEVELS - 1;
        this->loadLevel(this->startLevel);
    }
}

PlayState::~PlayState() {
    if (this->campaignProfile) { delete this->campaignProfile; this->campaignProfile = nullptr; }
    if (this->fractalNoise) { delete this->fractalNoise; this->fractalNoise = nullptr; }
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
// setCampaignProfile — set the noise profile type BEFORE the level loads
//
// Call this after creating PlayState but before the game loop starts.
// profileType must be one of: NOISE_AMPLIFIED, NOISE_FLAT, NOISE_NORMAL
// ─────────────────────────────────────────────────────────────────────────────

void PlayState::setCampaignProfile(int profileType) {
    if (profileType >= 0 && profileType < 3) {
        this->campaignProfileType = profileType;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// loadCampaignLevel — generate procedural terrain for Campaign Mode
//
// THIS IS THE SIMPLE VERSION. Here's how it works:
//
//   1. Create PerlinNoise with a seed (same seed = same terrain every time)
//   2. Create FractalNoise wrapping that PerlinNoise
//   3. Create a NoiseProfile (Amplified/Flat/Normal) via Factory pattern
//   4. Apply the profile to FractalNoise (sets amplitude, frequency, etc.)
//   5. Generate a heightmap array (one int per column = block height)
//   6. Build terrain from that heightmap using buildProceduralTerrain()
//   7. Clean up noise objects (they're temporary — only needed for generation)
//
// The terrain uses your EXISTING MountainBlock (dirt.png) system.
// No new rendering code. No new block types. Just a generated heightmap
// instead of a hardcoded one.
// ─────────────────────────────────────────────────────────────────────────────

void PlayState::loadCampaignLevel() {
    this->currentLevelIndex = 0;
    this->currentConfig = &CAMPAIGN_LEVEL;  // dedicated Perlin campaign config (infinite.png bg)

    // ── Reset scrolling and state ──
    this->scroll = 0.f;
    this->scrollY = 0.f;
    this->flyingTaraPhase = 0;
    this->submarineSpawned = false;
    this->flyingTaraClock.restart();
    this->levelTransitioning = false;
    this->levelTransitionTimer = 0.f;
    this->bossFelledTriggered = false;
    this->bossesSpawned = 0;
    this->bossesDefeated = 0;

    // ── Clear old enemies, vehicles, projectiles ──
    if (this->enemyManager) this->enemyManager->clearAll();
    if (this->enemyVehicleManager) this->enemyVehicleManager->clearAll();
    if (this->projectileManager) this->projectileManager->clearAll();

    // ── Delete old BlockManager ──
    if (this->blockManager) {
        delete this->blockManager;
        this->blockManager = nullptr;
    }

    // ── Create Perlin noise profile ──
    // Store it as a member so we can generate new columns for infinite scrolling.
    if (this->campaignProfile != nullptr) {
        delete this->campaignProfile;
        this->campaignProfile = nullptr;
    }
    this->campaignProfile = NoiseProfile::create(this->campaignProfileType);
    this->campaignProfile->setSeed(this->campaignSeed);

    // ── Create campaign Level via LevelManager ──
    // This replaces the survival Level with a 50x420 Perlin campaign Level.
    // The Level(NoiseProfile*) constructor generates the ENTIRE grid from
    // Perlin noise — ground, bedrock, water, biomes — all done internally.
    // We no longer need to build terrain via BlockManager at all.
    if (this->levelManager != nullptr) {
        this->levelManager->createCampaignLevel(this->campaignProfile);
    }

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;

    // ── Load background ──
    // Campaign uses infinite.png — tiled horizontally for infinite scrolling feel
    if (this->currentConfig != nullptr) {
        this->bgTex.loadFromFile(this->currentConfig->bgPath);
        this->bgSprite.setTexture(this->bgTex);
        float texH = static_cast<float>(this->bgTex.getSize().y);
        if (texH > 0.f) {
            // Scale to fill screen height, tile horizontally via render()
            this->bgScaleY = (float)SCREEN_H / texH;
            this->bgSprite.setScale(this->bgScaleY, this->bgScaleY);
        }
    }

    if (lvl == nullptr) {
        return;
    }

    // ── Load dirt texture and pass to Level for campaign rendering ──
    // Level::Draw() uses the dirt texture to render terrain blocks with
    // viewport culling. No MountainBlocks needed — avoids double-rendering
    // and collision corruption issues.
    this->texManager->loadTexture("dirt", "resources/Sprites/dirt.png");
    sf::Texture* dirtTex = &this->texManager->getTexture("dirt");
    lvl->setDirtTexture(dirtTex);

    // ── Create BlockManager (needed for destructible Block objects only) ──
    // For campaign mode, BlockManager is created but NOT used for terrain.
    // Terrain collision = Level::isSolid(), terrain rendering = Level::Draw().
    // BlockManager exists for future use (enemy-placed blocks, etc.)
    this->blockManager = new BlockManager(this->texManager, this->audManager, lvl);

    // ── Spawn player ON TOP of the Perlin terrain ──
    // getSurfaceRow() scans from top to find the first solid row.
    // Player Y = surface row * cellSize - offset (above the ground).
    if (this->characterManager != nullptr) {
        int cellSize = lvl->getCellSize();
        int worldOffX = lvl->getWorldOffX();
        float spawnWorldX = 200.f;

        // Convert world X to grid column: col = (worldX / cellSize) - worldOffX
        int spawnCol = static_cast<int>(spawnWorldX) / cellSize - worldOffX;
        if (spawnCol < 0) spawnCol = 0;
        if (spawnCol >= lvl->getWidth()) spawnCol = lvl->getWidth() - 1;

        int actualSurfaceRow = lvl->getSurfaceRow(spawnCol);
        float actualSurfaceY = (float)(actualSurfaceRow * cellSize);

        sf::Vector2f spawnPos(spawnWorldX, actualSurfaceY - 140.f);
        this->characterManager->initAllPositions(spawnPos);
    }

    // ── Set scrollY for this level ──
    // Campaign levels have vertical scroll enabled
    this->scrollY = 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// loadLevel — initializes (or re-initializes) everything for a given level
// (SURVIVAL MODE — uses predefined levels from LevelConfig)
// ─────────────────────────────────────────────────────────────────────────────

void PlayState::loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= TOTAL_LEVELS) return;

    this->currentLevelIndex = levelIndex;
    this->currentConfig = ALL_LEVELS[levelIndex];
    const LevelConfig* cfg = this->currentConfig;

    // ── Reset scrolling ──
    this->scroll = 0.f;
    this->scrollY = 0.f;

    // ── Reset vehicle/animation state ──
    this->flyingTaraPhase = 0;
    this->submarineSpawned = false;
    this->flyingTaraClock.restart();
    this->levelTransitioning = false;
    this->levelTransitionTimer = 0.f;
    this->bossFelledTriggered = false;
    this->bossesSpawned = 0;
    this->bossesDefeated = 0;

    // ── Clear old enemies, vehicles, projectiles ──
    if (this->enemyManager) this->enemyManager->clearAll();
    if (this->enemyVehicleManager) this->enemyVehicleManager->clearAll();
    if (this->projectileManager) this->projectileManager->clearAll();

    // ── Delete old BlockManager ──
    if (this->blockManager) {
        delete this->blockManager;
        this->blockManager = nullptr;
    }

    // ── Reset the Level grid ──
    if (this->levelManager) {
        Level* lvl = this->levelManager->getLevel();
        if (lvl != nullptr) {
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
            this->bgScaleY = (float)SCREEN_H / texH * 1.4f;
        }
        else {
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
                this->blockManager->buildGroundTerrain(surfaceRow, 3);
            }
            else {
                for (int rowOff = 0; rowOff < 3; rowOff++) {
                    int row = surfaceRow + rowOff;
                    if (row >= lvl->getHeight()) break;
                    for (int col = 0; col < lvl->getWidth(); col++) {
                        lvl->setSolid(row, col, true);
                    }
                }
            }

            if (cfg->hasMountain) {
                this->blockManager->buildMountainTerrain(4000.f, surfaceY);
            }

            // ── Reset player position ──
            if (this->characterManager != nullptr) {
                sf::Vector2f spawnPos(200.f, surfaceY - 140.f);
                this->characterManager->initAllPositions(spawnPos);
            }

            this->spawnPlatformsFromConfig();

            if (!cfg->isBossLevel) {
                this->spawnEnemiesFromConfig();
            }

            if (cfg->isBossLevel && this->enemyManager != nullptr) {
                int cellSize = lvl->getCellSize();
                int surfaceRow = lvl->getHeight() - 3;
                float surfaceY = (float)(surfaceRow * cellSize);
                float bossFootOffset = 360.f;

                if (cfg->bossType == ENEMY_BOSS_IRONOKAVA) {
                    float bossX = 1500.f;
                    this->enemyManager->spawnIronokava(bossX, surfaceY - bossFootOffset);
                    this->bossesSpawned = 1;
                }
            }

            if (cfg->isBossLevel && this->blockManager != nullptr && lvl != nullptr) {
                int cs = lvl->getCellSize();
                int surfaceRow = lvl->getHeight() - 3;
                float surfaceY = (float)(surfaceRow * cs);

                float poolLeft = 8350.f;
                float poolRight = 10125.f;
                int poolDepthBlocks = 20;
                float poolBottomY = surfaceY + (float)(poolDepthBlocks * cs);

                float leftWallX = poolLeft - (float)cs;
                for (int row = 0; row < poolDepthBlocks + 1; row++) {
                    float wy = surfaceY + (float)(row * cs);
                    this->blockManager->spawnIndestructibleBlock(leftWallX, wy);
                }

                float rightWallX = poolRight;
                for (int row = 0; row < poolDepthBlocks + 1; row++) {
                    float wy = surfaceY + (float)(row * cs);
                    this->blockManager->spawnIndestructibleBlock(rightWallX, wy);
                }

                int poolWidthBlocks = (int)((poolRight - poolLeft) / cs);
                for (int col = 0; col <= poolWidthBlocks; col++) {
                    float wx = poolLeft + (float)(col * cs);
                    this->blockManager->spawnIndestructibleBlock(wx, poolBottomY);
                }

                int stairSteps = poolDepthBlocks - 1;
                for (int step = 0; step < stairSteps; step++) {
                    float stairX = poolLeft + (float)(step * cs);
                    float stairY = surfaceY + (float)((step + 1) * cs);
                    this->blockManager->spawnIndestructibleBlock(stairX, stairY);
                }
            }

            if (cfg->hasWater) {
                this->waterShape.setPointCount(4);
                this->waterShape.setPoint(0, sf::Vector2f(cfg->waterX1, cfg->waterY1));
                this->waterShape.setPoint(1, sf::Vector2f(cfg->waterX2, cfg->waterY1));
                this->waterShape.setPoint(2, sf::Vector2f(cfg->waterX2, cfg->waterY2));
                this->waterShape.setPoint(3, sf::Vector2f(cfg->waterX1, cfg->waterY2));
                this->waterShape.setFillColor(Color(0, 30, 80, 140));
            }
            else {
                this->waterShape.setPointCount(4);
                this->waterShape.setPoint(0, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(1, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(2, sf::Vector2f(0.f, 0.f));
                this->waterShape.setPoint(3, sf::Vector2f(0.f, 0.f));
                this->waterShape.setFillColor(Color(0, 0, 0, 0));
            }

            if (!cfg->enableVerticalScroll) {
                this->scrollY = surfaceY - (float)SCREEN_H * 0.85f;
                if (this->scrollY < 0.f) this->scrollY = 0.f;
            }
        }
    }
}

void PlayState::spawnEnemiesFromConfig() {
    if (this->enemyManager == nullptr) return;
    if (this->currentConfig == nullptr) return;

    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
    if (lvl == nullptr) return;

    int cellSize = lvl->getCellSize();
    int surfaceRow = lvl->getHeight() - 3;
    float surfaceY = (float)(surfaceRow * cellSize);
    float rebelFootOffset = 140.f;

    float mtBaseX = 4000.f;
    float mtTop30 = surfaceY - 11.f * 48.f;
    float mtTop65 = surfaceY - 25.f * 48.f;
    float mtTop90 = surfaceY - 25.f * 48.f;

    const LevelConfig* cfg = this->currentConfig;

    for (int i = 0; i < cfg->enemyCount && i < 40; i++) {
        const EnemySpawnEntry* e = &cfg->enemies[i];
        float ex = e->x;
        float ey = e->y;

        if (ey == 0.f) {
            ey = surfaceY - rebelFootOffset;
        }

        if (ex == 0.f) {
            if (cfg->hasMountain) {
                if (i % 4 == 0) { ex = mtBaseX + 30.f * 48.f; ey = mtTop30 - rebelFootOffset; }
                else if (i % 4 == 1) { ex = mtBaseX + 90.f * 48.f; ey = mtTop90 - rebelFootOffset; }
                else if (i % 4 == 2) { ex = mtBaseX + 65.f * 48.f; ey = mtTop65 - rebelFootOffset; }
                else { ex = mtBaseX + 80.f * 48.f; ey = mtTop90 - rebelFootOffset; }
            }
            else {
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

void PlayState::checkLevelTransition() {
    // Campaign mode is infinite — no level transitions
    if (this->gameMode == MODE_CAMPAIGN) return;

    if (this->hud != nullptr && this->hud->isFelledShowing()) return;

    if (this->currentConfig != nullptr && this->currentConfig->isBossLevel) {
        if (this->enemyManager != nullptr && this->enemyManager->hasActiveBoss()) return;
        if (this->bossesSpawned < 2) return;
    }

    PlayerSoldier* player = this->characterManager
        ? this->characterManager->getCurrentCharacter() : nullptr;
    if (player == nullptr) return;

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

    if (playerX >= levelWidth - 200.f && !this->levelTransitioning) {
        this->levelTransitioning = true;
        this->levelTransitionTimer = 0.f;
    }

    if (this->levelTransitioning) {
        this->levelTransitionTimer += 1.f / 60.f;

        if (this->levelTransitionTimer >= 0.5f) {
            int nextLevel = this->currentLevelIndex + 1;

            if (nextLevel >= TOTAL_LEVELS) {
                if (this->stateManager != nullptr) {
                    int finalScore = this->scoreManager ? this->scoreManager->getScore() : 0;
                    this->stateManager->changeState(new GameOverState(finalScore));
                }
            }
            else {
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

    if (this->hud && this->enemyManager) {
        Boss* boss = this->enemyManager->getActiveBoss();
        if (boss != nullptr && (boss->isAlive() || boss->isDying())) {
            this->hud->setBossInfo(boss->getBossName(), boss->getHealthFraction());
        }
        else if (this->enemyManager->wasBossKilled()) {
            if (!this->bossFelledTriggered) {
                this->bossFelledTriggered = true;
                this->bossesDefeated++;
                const char* fallenName = this->enemyManager->getBossDiedName();
                if (fallenName == nullptr) fallenName = "UNKNOWN";
                this->hud->showBossFelled(fallenName);

                if (this->characterManager != nullptr) {
                    PlayerSoldier* currentPlayer = this->characterManager->getCurrentCharacter();
                    if (currentPlayer != nullptr) {
                        currentPlayer->healFullAndIncreaseHP(0);
                    }
                }

                if (this->currentConfig != nullptr && this->currentConfig->isBossLevel
                    && this->bossesDefeated == 1 && this->bossesSpawned == 1)
                {
                    Level* lvl = this->levelManager ? this->levelManager->getLevel() : nullptr;
                    if (lvl != nullptr) {
                        int cellSize = lvl->getCellSize();
                        int surfaceRow = lvl->getHeight() - 3;
                        float surfaceY = (float)(surfaceRow * cellSize);

                        float flyCenterX = 7000.f;
                        float flyCenterY = surfaceY - 400.f;
                        float bossX = flyCenterX;
                        float bossY = flyCenterY - 100.f;
                        this->enemyManager->spawnHairbuster(bossX, bossY, flyCenterX, flyCenterY);
                        this->bossesSpawned = 2;
                    }
                }
            }
            if (!this->hud->isFelledShowing()) {
                this->hud->clearBossInfo();
                if (this->enemyManager != nullptr) {
                    this->enemyManager->resetBossDied();
                }
                this->bossFelledTriggered = false;
            }
        }
    }

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

    // ── Update HUD (HP, score, weapon) AFTER damage is applied ──
    if (this->hud && this->characterManager) {
        this->hud->update(this->characterManager, this->currentLevelIndex + 1);
        if (this->scoreManager != nullptr) {
            this->hud->setScore(this->scoreManager->getScore());
            this->hud->setHighScore(this->scoreManager->getHighScore());
        }
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

    // ── INFINITE WORLD: advance the grid when the player approaches the right edge ──
    // In campaign mode, the Level grid is a sliding window over infinite Perlin terrain.
    // When the player gets near the right side, we shift the grid left and generate
    // new columns on the right. This makes the world scroll forever.
    if (this->gameMode == MODE_CAMPAIGN && player != nullptr && lvl != nullptr &&
        lvl->getCampaign() && this->campaignProfile != nullptr)
    {
        int cellSize = lvl->getCellSize();
        int worldOffX = lvl->getWorldOffX();
        float playerX = player->getPosition().x;

        // Right edge of the grid in world coordinates
        float gridRightWorldX = static_cast<float>((worldOffX + lvl->getWidth()) * cellSize);

        // Advance when player is within 15 columns of the right edge
        float advanceThreshold = static_cast<float>(15 * cellSize);
        if (playerX > gridRightWorldX - advanceThreshold) {
            // Advance by 20 columns at a time for efficiency
            lvl->advanceWorld(20, this->campaignProfile);
        }
    }

    if (player != nullptr && lvl != nullptr) {
        float playerX = player->getPosition().x;
        float levelWidth = 0.f;

        if (this->gameMode == MODE_CAMPAIGN && lvl->getCampaign()) {
            // Campaign is infinite — no right edge clamp
            // Use a very large effective width so scroll follows the player freely
            levelWidth = playerX + (float)SCREEN_W;  // always enough room ahead
        }
        else if (this->currentConfig != nullptr && this->currentConfig->levelWidth > 0.f) {
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
        if (lvl != nullptr) {
            int surfaceRow = lvl->getHeight() - 3;
            float surfaceY = (float)(surfaceRow * lvl->getCellSize());
            this->scrollY = surfaceY - (float)SCREEN_H * 0.85f;
            if (this->scrollY < 0.f) this->scrollY = 0.f;
        }
    }

    if (player != nullptr && this->currentConfig != nullptr && this->currentConfig->hasWater) {
        float px = player->getPosition().x;
        float py = player->getPosition().y;
        bool inWaterNow = (px >= this->currentConfig->waterX1 &&
            px <= this->currentConfig->waterX2 &&
            py >= this->currentConfig->waterY1 &&
            py <= this->currentConfig->waterY2);
        player->setInWater(inWaterNow);
    }
    else if (player != nullptr && lvl != nullptr && lvl->getCampaign()) {
        // Campaign mode: check if the player's grid cell is water ('w')
        int cellSize = lvl->getCellSize();
        int worldOffX = lvl->getWorldOffX();
        float px = player->getPosition().x;
        float py = player->getPosition().y;
        int col = static_cast<int>(px) / cellSize - worldOffX;
        int row = static_cast<int>(py) / cellSize;
        if (col >= 0 && col < lvl->getWidth() && row >= 0 && row < lvl->getHeight()) {
            char cell = lvl->getLvl()[row][col];
            player->setInWater(cell == 'w');
        }
        else {
            player->setInWater(false);
        }
    }
    else if (player != nullptr) {
        player->setInWater(false);
    }

    if (this->levelManager)
        this->levelManager->update(dt);

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

                int dir = (i % 2 == 0) ? DIR_RIGHT : DIR_LEFT;
                float spawnX = (dir == DIR_RIGHT)
                    ? player->getPosition().x - (float)SCREEN_W - 200.f
                    : player->getPosition().x + (float)SCREEN_W + 200.f;

                this->enemyVehicleManager->spawnFlyingTara(spawnX, taraY, dir);
                this->flyingTaraPhase = i + 1;
            }
        }
    }

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

    this->checkLevelTransition();
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
        bgY = 0.f;
    }

    if (bgX > 0.f) bgX = 0.f;

    if (this->currentConfig != nullptr && this->currentConfig->tileBg) {
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
            if (tileX + bgWidth < 0.f) continue;
            if (tileX > (float)SCREEN_W) break;

            this->bgSprite.setPosition(tileX, bgY);
            window.draw(this->bgSprite);
        }
    }
    else {
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

    if (this->levelTransitioning) {
        sf::RectangleShape overlay(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
        float alpha = this->levelTransitionTimer / 0.5f;
        if (alpha > 1.f) alpha = 1.f;
        overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(255.f * alpha)));
        overlay.setPosition(0.f, 0.f);
        window.draw(overlay);
    }

    if (this->hud)               this->hud->draw(window);
    this->renderBloodOverlay(window);
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
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Z) {
        if (this->characterManager != nullptr) {
            this->characterManager->switchCharacter();
        }
    }

    if (this->characterManager)
        this->characterManager->handleInput(event);
}

void PlayState::onEnter() {}

void PlayState::onExit() {
    if (this->audManager != nullptr) {
        this->audManager->stopMusic();
    }
}
