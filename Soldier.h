#pragma once
#include "DamagableEntity.h"
#include "TransformationState.h"


class Level;



class Soldier : public DamagableEntity {
protected:
    float velocityX;
    float velocityY;
    float maxVelocity;
    float baseMaxVelocity;
    int direction;  
    bool onGround;
    int lives;
    int currentHP;  
    int saturation;  
    float meleeDamage;
    float meleeCooldown;
    Clock meleeTimer;
    TransformationState* transformState;  

public:
    Soldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Soldier();

    virtual void update(float scroll, Level* lvl);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void takeDamage(int amount);
    void meleeAttack();
    int getState() const;  
    int getLives() const;
    int getCurrentHP() const;
    void respawn();
    void setTransformationState(TransformationState* newState);
    TransformationState* getTransformationState() const;

    
    float getBaseMaxVelocity() const { return this->baseMaxVelocity; }
    void setBaseMaxVelocity(float val) { this->baseMaxVelocity = val; }
    float getMaxVelocity() const { return this->maxVelocity; }
    void setMaxVelocity(float val) { this->maxVelocity = val; }

    
    void setDirectionAndVelocity(int dir);
    void decelerate();

    
    void handleJump();

    
    
    void resolveBlockCollisions(DamagableEntity** blocks, int count);

protected:
    virtual void applyGravity();
    virtual void handleCollision(Level* lvl);
    virtual void applyMovement(float& scroll);
    virtual void handleStateTimers();
    virtual void onDeath() = 0;
};