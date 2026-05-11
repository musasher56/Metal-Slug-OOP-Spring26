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
class FractalNoise;
class NoiseProfile;

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

    Texture    bloodOverlayTex;
    Sprite     bloodOverlaySprite;
    GameStateManager* stateManager;

    Clock flyingTaraClock;
    int   flyingTaraPhase;
    bool  submarineSpawned;

    sf::ConvexShape waterShape;
    float waterBaseY;

    int startLevel;
    int currentLevelIndex;
    const LevelConfig* currentConfig;
    bool levelTransitioning;
    float levelTransitionTimer;
    bool hudVisible;
    bool bossFelledTriggered;
    int  bossesSpawned;
    int  bossesDefeated;

    FractalNoise* fractalNoise;       
    NoiseProfile* campaignProfile;  
    int  campaignSeed;               
    int  campaignProfileType;        

public:
    PlayState(int mode, int startChar, TextureManager* texMgr, AudioManager* audMgr, int startLvl = 0);
    virtual ~PlayState();
    void setStateManager(GameStateManager* mgr) { this->stateManager = mgr; }
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();

    void setCampaignProfile(int profileType);

private:
    void renderBloodOverlay(RenderWindow& window);
    void loadLevel(int levelIndex);
    void loadCampaignLevel();        
    void checkLevelTransition();
    void spawnEnemiesFromConfig();
    void spawnPlatformsFromConfig();
};
