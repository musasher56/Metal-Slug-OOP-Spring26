#pragma once
#include "GameState.h"
#include "ProjectileManager.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "BlockManager.h"
#include <SFML/Graphics.hpp>

class CharacterManager;
class LevelManager;
class ScoreManager;
class HUD;

class PlayState : public GameState {
private:
    void* entityManager;
    void* enemyManager;
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
    bool  movingLeft;
    bool  movingRight;

    Texture bgTex;
    Sprite  bgSprite;
    float   bgScaleY;

    RenderWindow* gameWindow;
    sf::Vector2f   lastMouseWorld;

    bool       debugMode;
    sf::Font   debugFont;
    sf::Text   debugText;

public:
    PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayState();

    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();

private:
    void renderDebug(RenderWindow& window);
    void spawnTestBlocks();
};