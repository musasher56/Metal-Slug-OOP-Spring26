#pragma once
#include "GameState.h"
#include "ProjectileManager.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "BlockManager.h"
#include "EnemyManager.h"
#include "EnemyVehicleManager.h"
#include "LevelConfig.h"
#include <SFML/Graphics.hpp>

class CharacterManager;
class GameStateManager;
class LevelManager;
class ScoreManager;
class HUD;

class PlayState : public GameState {
private:
    void* entityManager;
    EnemyManager* enemyManager;
    EnemyVehicleManager* enemyVehicleManager;
    ProjectileManager* projectileManager;
    void* collectibleManager;

    TextureManager* texManager;
    AudioManager* audManager;

    Level* level;
    CharacterManager* characterManager;
    LevelManager* levelManager;
    ScoreManager* scoreManager;
    HUD* hud;
    BlockManager* blockManager;

    int   gameMode;
    float scroll;
    float scrollY;
    bool  movingLeft;
    bool  movingRight;
    Texture bgTex;
    Sprite  bgSprite;
    float   bgScaleY;

    RenderWindow* gameWindow;
    sf::Vector2f   lastMouseWorld;

    bool       debugMode;
    bool       showHitboxes;
    sf::Font   debugFont;
    sf::Text   debugText;

    // ── Developer / God Mode (G key) ─────────────────────────────────────
    // Single press of G toggles immortality ON/OFF.
    // devKeyTimer drives the pulsing animation on the on-screen banner.
    bool       devModeActive;
    sf::Clock  devKeyTimer;

    Texture    bloodOverlayTex;
    Sprite     bloodOverlaySprite;
    GameStateManager* stateManager;

    Clock flyingTaraClock;
    int   flyingTaraPhase;
    bool  submarineSpawned;

    // Water pool overlay
    sf::ConvexShape waterShape;
    float waterBaseY;

    // ── Level management ──
    int startLevel;               // which level to begin at (0 = campaign, 1/2 = skip ahead)
    int currentLevelIndex;
    const LevelConfig* currentConfig;
    bool levelTransitioning;
    float levelTransitionTimer;
    bool hudVisible;              // T key: toggles HUD + debug panel + hitboxes together
    bool bossFelledTriggered;     // true once the "GREAT ENEMY FELLED" message has been shown
    int  bossesSpawned;           // how many bosses have been spawned in the current boss level
    int  bossesDefeated;          // how many bosses have been defeated in the current boss level

public:
    PlayState(int mode, int startChar, TextureManager* texMgr, AudioManager* audMgr, int startLvl = 0);
    virtual ~PlayState();
    void setStateManager(GameStateManager* mgr) { this->stateManager = mgr; }
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();

private:
    void renderDebug(RenderWindow& window);
    void renderHitboxes(RenderWindow& window);
    void renderBloodOverlay(RenderWindow& window);
    void spawnTestBlocks();
    void spawnTestEnemies();
    void loadLevel(int levelIndex);
    void checkLevelTransition();
    void spawnEnemiesFromConfig();
    void spawnPlatformsFromConfig();
};