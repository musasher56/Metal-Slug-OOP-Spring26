#pragma once
#include "GameState.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "Level.h"
#include "PlayerSoldier.h"

// Forward declarations for managers (stubbed as void* for now)
class CharacterManager;
class LevelManager;
class ScoreManager;
class HUD;

// WHY: PlayState is the main gameplay state - owns all game managers and entities
class PlayState : public GameState {
private:
    // Direct members (always alive):
    void* entityManager;         // EntityManager* - forward declared as void to avoid circular deps
    void* enemyManager;          // EnemyManager*
    void* enemyVehicleManager;   // EnemyVehicleManager*
    void* projectileManager;     // ProjectileManager*
    void* collectibleManager;    // CollectibleManager*
    
    // WHY: Temporary members for basic gameplay until managers are implemented
    TextureManager* texManager;  // Reference only, owned by Game
    AudioManager* audManager;    // Reference only, owned by Game
    Level* level;                // Owned by PlayState
    CharacterManager* characterManager; // Owns up to 4 PlayerSoldiers
    LevelManager* levelManager;  // Manages levels
    ScoreManager* scoreManager;  // Tracks score
    HUD* hud;                    // Heads-up display
    
    Sprite bgSprite;             // Background sprite for gameplay
    Texture bgTex;               // Background texture
    
    int gameMode;  // MODE_SURVIVAL, MODE_CAMPAIGN, MODE_SELF_PLAY
    bool movingLeft;
    bool movingRight;
    
public:
    PlayState(int mode, TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayState();
    
    virtual void update(float dt);
    virtual void render(RenderWindow& window);
    virtual void handleEvent(Event& event);
    virtual void onEnter();
    virtual void onExit();
};
