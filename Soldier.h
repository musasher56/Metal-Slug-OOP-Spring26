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
    Clock invincibilityClock;
    bool  isInvincible;

    // Physical collision extent in TEXTURE PIXELS (before scale).
    // handleCollision() multiplies these by the sprite's current scale to get
    // the world-space collision box dimensions.  Every subclass constructor
    // sets these to match its actual sprite frame size so collision height
    // equals visual height — preventing float-above-ground when switching
    // characters who use different scales.
    //
    // Default: 34 x 40 (matches the original hardcoded values for Marco).
    int physW;
    int physH;

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
    bool getIsInvincible() const { return this->isInvincible; }
    void respawn();
    void setTransformationState(TransformationState* newState);
    TransformationState* getTransformationState() const;


    float getBaseMaxVelocity() const { return this->baseMaxVelocity; }
    void setBaseMaxVelocity(float val) { this->baseMaxVelocity = val; }
    float getMaxVelocity() const { return this->maxVelocity; }
    void setMaxVelocity(float val) { this->maxVelocity = val; }


    void setDirectionAndVelocity(int dir);
    void decelerate();

    // Copies world position, velocity, direction, and onGround flag from
    // another Soldier. Used by CharacterManager on character switch so the
    // incoming character seamlessly inherits the outgoing one's physics state
    // instead of teleporting to its constructor-default (200, 300).
    void copyPhysicsFrom(Soldier* other);


    void handleJump();

    bool inWater;
    void setInWater(bool val);
    bool getInWater() const;



    void resolveBlockCollisions(DamagableEntity** blocks, int count);

protected:
    virtual void applyGravity();
    virtual void applyWaterPhysics();
    virtual void handleCollision(Level* lvl);
    virtual void applyMovement(float& scroll);
    virtual void handleStateTimers();
    virtual void onDeath() = 0;
};