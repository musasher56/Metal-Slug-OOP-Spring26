#pragma once
#include "DamagableEntity.h"
#include "TransformationState.h"

// WHY: Soldier extends DamagableEntity with movement, gravity, lives, transformation state
// Base class for PlayerSoldier and Enemy - handles physics, collision, state timers
class Soldier : public DamagableEntity {
protected:
    float velocityX;
    float velocityY;
    float maxVelocity;
    float baseMaxVelocity;
    int direction;  // DIR_LEFT=0, DIR_RIGHT=1
    bool onGround;
    int lives;
    int currentHP;  // 3=HEALTHY, 2=INJURED, 1=CRITICAL, 0=DEAD
    int saturation;  // replenished by Food collectibles
    float meleeDamage;
    float meleeCooldown;
    Clock meleeTimer;
    TransformationState* transformState;  // State pattern - current transformation

public:
    Soldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Soldier();
    
    virtual void update(float scroll, void* lvl) override;
    virtual void draw(RenderWindow& window, float scroll) override;
    virtual void takeDamage(int amount) override;
    void meleeAttack();
    int getState() const;  // returns currentHP as state
    int getLives() const;
    int getCurrentHP() const;
    void respawn();
    void setTransformationState(TransformationState* newState);
    TransformationState* getTransformationState() const;
    
    // WHY: Provide protected getters/setters for velocity to allow TransformationState access
    float getBaseMaxVelocity() const { return this->baseMaxVelocity; }
    void setBaseMaxVelocity(float val) { this->baseMaxVelocity = val; }
    float getMaxVelocity() const { return this->maxVelocity; }
    void setMaxVelocity(float val) { this->maxVelocity = val; }
    
protected:
    virtual void handleJump();
    virtual void applyGravity();
    virtual void handleCollision(void* lvl);  // lvl is Level*
    virtual void applyMovement(float& scroll);
    virtual void handleStateTimers();
    virtual void onDeath() override = 0;
};
