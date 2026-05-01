#pragma once
#include "Entity.h"

class EnemyManager;
class CharacterManager;
class Level;
class ProjectileManager;   // forward declare — needed for friend below

class Projectile : public Entity {
    // WHY friend?
    // ProjectileManager is the factory that creates and configures
    // projectiles.  Instead of making damage/fromEnemy/blastRadius
    // public (visible to every class), we grant access only to the one
    // class that legitimately needs it.  friend is the correct tool for
    // tightly coupled collaborator relationships in C++.
    friend class ProjectileManager;

protected:
    float velocityX;
    float velocityY;
    int   damage;
    bool  fromEnemy;
    bool  isExplosive;
    int   blastRadius;
    int   projectileClass;

public:
    Projectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Projectile();

    // Template Method — sealed pipeline, NOT overridden by subclasses.
    // Order: move() → checkTileCollision() → checkBounds()
    virtual void update(float scroll, Level* lvl) override;
    virtual void draw(RenderWindow& window, float scroll) override;

    IntRect getBoundingBox() const;
    int     getDamage()      const;
    bool    isFromEnemy()    const;

    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
    void         setVelocity(float vx, float vy);

protected:
    virtual void move(float scroll) = 0;  // subclass defines motion only

private:
    void checkTileCollision(Level* lvl);
    void checkBounds(float scroll);
};

// ============================================================
// StraightProjectile — constant velocity
// ============================================================
class StraightProjectile : public Projectile {
private:
    float angle;
public:
    StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang);
    virtual ~StraightProjectile();
protected:
    virtual void move(float scroll) override;
};

// ============================================================
// BallisticProjectile — parabolic arc (gravity accumulates)
// ============================================================
class BallisticProjectile : public Projectile {
protected:
    float gravity;
public:
    BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BallisticProjectile();
protected:
    virtual void move(float scroll) override;
};

// ============================================================
// ExplosiveProjectile — ballistic + area damage on impact
// ============================================================
class ExplosiveProjectile : public BallisticProjectile {
public:
    ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ExplosiveProjectile();
    virtual void onImpact(EnemyManager* em, CharacterManager* cm) override;
};