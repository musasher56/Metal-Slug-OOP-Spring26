#pragma once
#include "GameState.h"

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
    
    int gameMode;  // MODE_SURVIVAL, MODE_CAMPAIGN, MODE_SELF_PLAY
    
public:
    PlayState(int mode);
    virtual ~PlayState();
    
    virtual void update(float dt) override;
    virtual void render(RenderWindow& window) override;
    virtual void handleEvent(Event& event) override;
    virtual void onEnter() override;
    virtual void onExit() override;
};
