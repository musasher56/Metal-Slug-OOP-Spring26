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