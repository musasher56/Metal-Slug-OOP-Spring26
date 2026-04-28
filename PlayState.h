#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "Level.h"
#include "Player.h"

// WHY: PlayState is the main gameplay state - owns all game managers and entities
class PlayState : public GameState {
private:
    // Direct members (always alive):
    void* entityManager;         // EntityManager* - forward declared as void to avoid circular deps
    void* levelManager;          // LevelManager*
    void* characterManager;      // CharacterManager*
    void* enemyManager;          // EnemyManager*
    void* enemyVehicleManager;   // EnemyVehicleManager*
    void* projectileManager;     // ProjectileManager*
    void* collectibleManager;    // CollectibleManager*
    void* scoreManager;          // ScoreManager*
    void* hud;                   // HUD*
    
    // WHY: Temporary members for basic gameplay until managers are implemented
    TextureManager* texManager;  // Reference only, owned by Game
    AudioManager* audManager;    // Reference only, owned by Game
    Level* level;                // Owned by PlayState
    Player* player;              // Owned by PlayState
    Sprite bgSprite;             // Background sprite for gameplay
    Texture bgTex;               // Background texture
    
    int gameMode;  // MODE_SURVIVAL, MODE_CAMPAIGN, MODE_SELF_PLAY
    bool movingLeft;
    bool movingRight;
    
public:
    PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayState();
    
    virtual void update(float dt) override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event) override;
    virtual void onEnter() override;
    virtual void onExit() override;
};
