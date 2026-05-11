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

    
    
    void checkTileCollision(Level* lvl);
    void checkBounds(float scrollX, float scrollY);
};







class StraightProjectile : public Projectile {
private:
    float angle;    
public:
    StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang);
    virtual ~StraightProjectile();

    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    virtual void move(float scroll);
};









class BallisticProjectile : public Projectile {
protected:
    float gravity;   
public:
    BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BallisticProjectile();

    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    virtual void move(float scroll);
};










class ExplosiveProjectile : public BallisticProjectile {
public:
    ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ExplosiveProjectile();

    
    
    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
};



















class FlameParticle : public StraightProjectile {
private:
    int lifetime;       
    int maxLifetime;    
public:
    FlameParticle(TextureManager* texMgr, AudioManager* audMgr,
                  float ang, int frames = 20);
    virtual ~FlameParticle();

    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    
    
    
    virtual void move(float scroll);
};





















class LaserBeam : public StraightProjectile {
private:
    int lifetime;    
    int beamDir;     
public:
    LaserBeam(TextureManager* texMgr, AudioManager* audMgr,
              int dir, int frames = 5);
    virtual ~LaserBeam();

    
    
    virtual IntRect getBoundingBox() const;

    
    virtual void update(float scroll, Level* lvl);

    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    
    virtual void move(float scroll);
};