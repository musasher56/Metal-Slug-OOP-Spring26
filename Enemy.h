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
    int paraState;       // 0 = flying (parachute), 1 = grounded (rebel)
    float landY;         // target Y where landing completes
    float fallSpeed;     // slow descent speed while parachuting
    Animation flyAnim;   // single-frame parachute animation
    float swayTimer;     // timer for gentle left-right sway
    bool startDescent;   // true once player is near mountain height
    float triggerX;      // X position that triggers descent when player crosses it

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