#include "PlayState.h"
#include "GameStateManager.h"
#include "GameOverState.h"
#include "CharacterManager.h"
#include "LevelManager.h"
#include "ScoreManager.h"
#include "HUD.h"
#include "BlockManager.h"
#include "DamagableEntity.h"
#include <cstdio>



PlayState::PlayState(int mode, int startChar, TextureManager* texMgr, AudioManager* audMgr)
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
{
    this->id = GSTATE_PLAY;

    this->levelManager = new LevelManager();

    this->characterManager = new CharacterManager(texMgr, audMgr, startChar);
    this->scoreManager = new ScoreManager();
    this->hud = new HUD();
    this->projectileManager = new ProjectileManager(texMgr, audMgr);
    this->enemyManager = new EnemyManager(texMgr, audMgr);
    this->enemyManager->setProjectileManager(this->projectileManager);

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

    this->scroll = 0.f;
    this->scrollY = 0.f;
    this->bgTex.loadFromFile("resources/Sprites/background.png");
    this->bgSprite.setTexture(this->bgTex);

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
            int groundRow = lvl->getHeight() - 1;
            int surfaceRow = groundRow - 2;
            float surfaceY = (float)(surfaceRow * cellSize);




            this->blockManager->buildGroundTerrain(surfaceRow, 3);


            this->blockManager->buildMountainTerrain(4000.f, surfaceY);

            if (this->characterManager != nullptr) {
                sf::Vector2f spawnPos(200.f, surfaceY - 140.f);
                this->characterManager->initAllPositions(spawnPos);
            }

            this->spawnTestBlocks();
            this->spawnTestEnemies();
        }
    }
}

PlayState::~PlayState() {
    if (this->enemyManager) { delete this->enemyManager; this->enemyManager = nullptr; }
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

    if (this->projectileManager && player != nullptr) {
        this->projectileManager->checkEnemyBulletHitPlayer(player);
    }

    if (this->projectileManager && lvl != nullptr)
        this->projectileManager->postEntityUpdate(this->scroll, this->scrollY, lvl);

    if (this->enemyManager)
        this->enemyManager->cleanup();

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
        int surfaceRow = lvl->getHeight() - 3;
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
    if (this->enemyManager)      this->enemyManager->draw(window, this->scroll, this->scrollY);
    if (this->characterManager)  this->characterManager->draw(window, this->scroll, this->scrollY);
    if (this->projectileManager) this->projectileManager->draw(window, this->scroll, this->scrollY);
    if (this->hud)               this->hud->draw(window);
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

    sprintf(line, "Projectiles: %d\n",
        this->projectileManager ? this->projectileManager->getActiveCount() : -1);
    append(line);
    sprintf(line, "Blocks: %d / %d  Enemies: %d\n",
        this->blockManager ? this->blockManager->getActiveCount() : -1,
        this->blockManager ? this->blockManager->getTotalCount() : -1,
        this->enemyManager ? this->enemyManager->getActiveCount() : -1);
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
    sprintf(line, "X=Shoot  C=Grenade  Arrows=Move  Space=Jump  Z=Switch  H=Hitboxes");
    append(line);

    this->debugText.setString(buf);
    RectangleShape bg(sf::Vector2f(400.f, 130.f));
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

    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Z) {
        if (this->characterManager != nullptr) {
            this->characterManager->switchCharacter();
        }
    }

    if (this->characterManager)
        this->characterManager->handleInput(event);
}

void PlayState::onEnter() {}
void PlayState::onExit() {}

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

    // ── Ground level enemies ──────────────────────────────────────────────
    this->enemyManager->spawnMartian(15.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnRebel(35.f * 48.f, surfaceY - rebelFootOffset);

    // ── Platform enemies ─────────────────────────────────────────────────
    // Platform 1: col 8-15, row 32
    float platY1 = 32.f * 48.f;
    // Platforms 2,3,4: col 22-28 / 32-35 / 42-51, row 30
    float platY2 = 30.f * 48.f;

    this->enemyManager->spawnRebel(10.f * 48.f, platY1 - rebelFootOffset);
    this->enemyManager->spawnRebel(24.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnRebel(34.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnRebel(48.f * 48.f, platY2 - rebelFootOffset);

    // ── Mountain enemies ─────────────────────────────────────────────────
    float mtBaseX = 4000.f;
    float mtTop30 = surfaceY - 11.f * 48.f;
    float mtTop65 = surfaceY - 25.f * 48.f;
    float mtTop90 = surfaceY - 25.f * 48.f;
    float mtTop140 = surfaceY - 19.f * 48.f;
    this->enemyManager->spawnRebel(mtBaseX + 30.f * 48.f, mtTop30 - rebelFootOffset);
    this->enemyManager->spawnRebel(mtBaseX + 90.f * 48.f, mtTop90 - rebelFootOffset);
    this->enemyManager->spawnRebel(mtBaseX + 140.f * 48.f, mtTop140 - rebelFootOffset);

    // ── Bazooka soldiers — longer range, explosive rockets ───────────────
    this->enemyManager->spawnBazooka(25.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnBazooka(50.f * 48.f, surfaceY - rebelFootOffset);

    // Bazooka on the mountain peak — rockets from high ground (disabled)
    ///this->enemyManager->spawnBazooka(mtBaseX + 65.f * 48.f, mtTop65 - rebelFootOffset);

    // ── Shielded soldiers — block paths, shield absorbs 3 frontal hits ───
    this->enemyManager->spawnShielded(20.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnShielded(40.f * 48.f, surfaceY - rebelFootOffset);
    this->enemyManager->spawnShielded(mtBaseX + 90.f * 48.f, mtTop90 - rebelFootOffset);

    // ── Grenade soldiers — lob grenades from elevated positions ──────────
    this->enemyManager->spawnGrenade(12.f * 48.f, platY1 - rebelFootOffset);
    this->enemyManager->spawnGrenade(33.f * 48.f, platY2 - rebelFootOffset);
    this->enemyManager->spawnGrenade(mtBaseX + 30.f * 48.f, mtTop30 - rebelFootOffset);

    // ── Martians — rapid energy blasts, high value targets ───────────────
    float mtPeak = surfaceY - 25.f * 48.f;
    this->enemyManager->spawnMartian(mtBaseX + 140.f * 48.f, mtPeak - rebelFootOffset);
    this->enemyManager->spawnMartian(mtBaseX + 65.f * 48.f, mtTop65 - rebelFootOffset);
}