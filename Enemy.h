#pragma once
#include "Soldier.h"
#include "ProjectileManager.h"

class PlayerSoldier;

class Enemy : public Soldier {
protected:
    int aiState;
    float detectionRange;
    float attackRange;
    float attackCooldown;
    Clock attackTimer;
    ProjectileManager* pm;
    bool activated;
    float patrolCenter;
    float patrolRadius;
    int enemyType;

    Animation walkAnim;
    Animation shootAnim;
    Animation deathAnim;
    Animation* currentAnim;

    int frameW;
    int frameH;
    int walkFrames;
    int shootFrames;
    int deathFrames;
    bool dying;

    int baseFrameW;
    int baseFrameH;
    Clock deathTimer;
    float deathDuration;

    float deathSpriteScale;
    bool faceRight;

    void switchAnim(Animation* newAnim);
    void applyDirectionFlip();

public:
    Enemy(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Enemy();

    void setProjectileManager(ProjectileManager* manager);
    void setPatrol(float centerX, float radius);
    void setEnemyType(int type);

    bool isActivated() const;
    bool isDying() const;
    float distanceTo(PlayerSoldier* player) const;
    bool playerInRange(PlayerSoldier* player, float range) const;

    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    void updateBoundingBox();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);

    int getAIState() const;
    int getEnemyType() const;
    void takeDamage(int amount);
};

class RebelSoldier : public Enemy {
public:
    RebelSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~RebelSoldier();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
};

class BazookaSoldier : public Enemy {
public:
    BazookaSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BazookaSoldier();
    virtual void performAttack(PlayerSoldier* player);
};

class ShieldedSoldier : public Enemy {
private:
    bool hasShield;
    int  shieldHP;

public:
    ShieldedSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ShieldedSoldier();
    virtual void performAttack(PlayerSoldier* player);
    virtual void takeDamageFrom(int amount, int bulletDir);
};

class GrenadeSoldier : public Enemy {
public:
    GrenadeSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~GrenadeSoldier();
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Martian : public Enemy {
private:
    bool inPodPhase;
    int  podHP;

public:
    Martian(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Martian();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Paratrooper : public Enemy {
private:
    int paraState;
    float landY;
    float fallSpeed;
    Animation flyAnim;
    float swayTimer;
    bool startDescent;
    float triggerX;

protected:
    virtual void applyGravity();

public:
    Paratrooper(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Paratrooper();
    void setLandY(float y);
    void setTriggerX(float x);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};

class Boss : public Enemy {
protected:
    int   bossPhase;
    float phase2Threshold;
    const char* bossName;
    bool  entranceDone;
    float entranceTimer;

    Animation idleAnim;
    Animation chargeAnim;
    Animation specialAnim;

    float specialCooldown;
    Clock specialTimer;
    float chargeCooldown;
    Clock chargeTimer;
    float chargeSpeed;
    bool  isCharging;
    float chargeDuration;
    float chargeElapsed;

public:
    Boss(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Boss();

    const char* getBossName() const;
    float getHealthFraction() const;
    bool  isEntranceDone() const;

    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Ironokava : public Boss {
public:
    Ironokava(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Ironokava();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Hairbuster : public Boss {
private:
    float flyCenterX;
    float flyCenterY;
    float flyRadiusX;
    float flyRadiusY;
    float flyAngle;
    float flySpeed;
    float diveTargetX;
    float diveTargetY;
    bool  isDiving;
    float diveSpeed;
    float diveTimer;
    float diveDuration;
    float bombCooldown;
    Clock bombTimer;

protected:
    virtual void applyGravity();

public:
    Hairbuster(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Hairbuster();
    void  setFlyCenter(float cx, float cy);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};

class SeaSatan : public Boss {
private:
    float swimCenterX;
    float surfaceY;
    float floatBaseY;
    float bobAngle;
    float bobSpeed;
    float bobAmplitude;
    float patrolLeftX;
    float patrolRightX;
    float moveDir;
    float patrolSpeed;
    float torpedoCooldown;
    Clock torpedoTimer;
    float depthChargeCooldown;
    Clock depthChargeTimer;
    float sonarPulseCooldown;
    Clock sonarPulseTimer;
    int   sonarPulseCount;

    Animation swimAnim;

protected:
    virtual void applyGravity();

public:
    SeaSatan(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~SeaSatan();
    void  setSwimCenter(float cx, float cy, float surfY);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};

class Sherry : public Boss {
private:
    // Multi-phase animation system
    // Phase 0: Sitting on throne (sherry-onthrone.png)
    // Phase 1: Getting up animation (sherry-gettingup.png, 4 frames)
    // Phase 2: Stood up briefly (sherry-stoodup.png) + empty throne shown
    // Phase 3: Walking (sherry-walk.png, 5 frames) with attacks

    int   sherryPhase;
    float phaseTimer;

    // Throne position (where the empty throne stays)
    float throneX;
    float throneY;
    bool  throneVisible;

    // Animations for each phase
    Animation throneAnim;       // sitting on throne (1 frame)
    Animation getupAnim;        // getting up (4 frames)
    Animation stoodupAnim;      // stood up static (1 frame)
    Animation sherryWalkAnim;   // walking (5 frames) - Sherry-specific, avoids clash with Enemy::walkAnim

    // Empty throne sprite drawn separately
    Sprite emptyThroneSprite;
    Texture* emptyThroneTex;

    // Attack timers
    float laserCooldown;
    Clock  laserTimer;
    float  missileCooldown;
    Clock  missileTimer;
    float  missileBurstCooldown;
    Clock  missileBurstTimer;
    int    missileBurstCount;
    int    missileBurstMax;
    float  missileBurstInterval;
    Clock  missileBurstClock;
    bool   inMissileBurst;

    // Laser attack state
    bool  isFiringLaser;
    float laserDuration;
    float laserElapsed;
    float laserAngle;  // angle toward player when laser fires
    bool  laserDamageApplied;  // prevent multi-frame damage

    // Patrol bounds for walking phase
    float patrolLeftX;
    float patrolRightX;
    float patrolDir;

protected:
    virtual void applyGravity();

public:
    Sherry(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Sherry();
    void  setThronePosition(float x, float y);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};
