#pragma once
#include "Entity.h"

// Forward declarations
class EnemyManager;
class CharacterManager;

// WHY: Projectile is the abstract base class for all projectiles
// Extends Entity with velocity, damage, and blast properties
class Projectile : public Entity {
protected:
    float velocityX;
    float velocityY;
    int damage;
    bool fromEnemy;
    bool isExplosive;
    int blastRadius;
    int projectileClass;  // ProjectileClass constant

public:
    Projectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Projectile();

    virtual void update(float scroll) = 0;
    virtual void draw(RenderWindow& window, float scroll) override;
    
    IntRect getBoundingBox() const;
    int getDamage() const;
    bool isFromEnemy() const;
    
    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
};

// ========== StraightProjectile : Projectile ==========
// Constant-velocity linear trajectory. Used by Pistol, HMG, Martian energy beam.
class StraightProjectile : public Projectile {
private:
    float angle;

public:
    StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang);
    virtual ~StraightProjectile();

    virtual void update(float scroll) override;
};

// ========== BallisticProjectile : Projectile ==========
// Parabolic arc under simulated gravity. Used by RocketLauncher, Bazooka, grenades.
class BallisticProjectile : public Projectile {
protected:
    float gravity;
    float initialVelocityX;
    float initialVelocityY;
    float timeAlive;

public:
    BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BallisticProjectile();

    virtual void update(float scroll) override;
};

// ========== ExplosiveProjectile : BallisticProjectile ==========
// Explodes on impact. Applies blast damage in radius.
class ExplosiveProjectile : public BallisticProjectile {
public:
    ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ExplosiveProjectile();

    virtual void onImpact(EnemyManager* em, CharacterManager* cm) override;
};
