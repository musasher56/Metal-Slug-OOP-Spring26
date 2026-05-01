#pragma once
#include "GameState.h"
#include "Projectilemanager.h"    // filename on disk: Projectilemanager.h (lowercase m)
#include "PlayerSoldier.h"
#include "Level.h"
#include <SFML/Graphics.hpp>

class CharacterManager;
class LevelManager;
class ScoreManager;
class HUD;

class PlayState : public GameState {
private:
    void*              entityManager;
    void*              enemyManager;
    void*              enemyVehicleManager;
    ProjectileManager* projectileManager;   // type = ProjectileManager (uppercase M)
    void*              collectibleManager;  // variable = projectileManager (lowercase p)

    TextureManager*    texManager;
    AudioManager*      audManager;

    Level*             level;
    CharacterManager*  characterManager;
    LevelManager*      levelManager;
    ScoreManager*      scoreManager;
    HUD*               hud;

    int   gameMode;
    float scroll;
    bool  movingLeft;
    bool  movingRight;

    Texture bgTex;
    Sprite  bgSprite;
    float   bgScaleY;

    RenderWindow*  gameWindow;
    sf::Vector2f   lastMouseWorld;

    bool       debugMode;
    sf::Font   debugFont;
    sf::Text   debugText;

public:
    PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayState();

    virtual void update(float dt)             override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event)    override;
    virtual void onEnter()                    override;
    virtual void onExit()                     override;

private:
    void renderDebug(RenderWindow& window);
};