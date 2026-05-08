#pragma once
#include "GameState.h"
#include "ProjectileManager.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "BlockManager.h"
#include "EnemyManager.h"
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
    void* enemyVehicleManager;
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

    Texture    bloodOverlayTex;
    Sprite     bloodOverlaySprite;
    GameStateManager* stateManager;

public:
    PlayState(int mode, int startChar, TextureManager* texMgr, AudioManager* audMgr);
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
};