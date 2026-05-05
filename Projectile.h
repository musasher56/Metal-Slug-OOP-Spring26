#pragma once
#include "Entity.h"

class EnemyManager;
class CharacterManager;
class Level;
class ProjectileManager;

class Projectile : public Entity {
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

    virtual void update(float scroll, Level* lvl);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    IntRect getBoundingBox() const;
    int     getDamage()      const;
    bool    isFromEnemy()    const;

    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
    void         setVelocity(float vx, float vy);

protected:
    virtual void move(float scroll) = 0;

private:
    void checkTileCollision(Level* lvl);
    // FIX: added scrollY so Y bounds use camera position, not SCREEN_H
    void checkBounds(float scrollX, float scrollY);
};

class StraightProjectile : public Projectile {
private:
    float angle;
public:
    StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang);
    virtual ~StraightProjectile();
protected:
    virtual void move(float scroll);
};

class BallisticProjectile : public Projectile {
protected:
    float gravity;
public:
    BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BallisticProjectile();
protected:
    virtual void move(float scroll);
};

class ExplosiveProjectile : public BallisticProjectile {
public:
    ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ExplosiveProjectile();
    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
};