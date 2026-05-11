#pragma once
#include "DamagableEntity.h"
#include "Animation.h"

class PlayerSoldier;
class ProjectileManager;
class Level;


class EnemyVehicle : public DamagableEntity {
protected:
    int biome;
    ProjectileManager* pm;
    bool destroyed;
    bool faceRight;
    float velocityX;
    float velocityY;
    Animation anim;
    Animation* currentAnim;

public:
    EnemyVehicle(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~EnemyVehicle();

    void setProjectileManager(ProjectileManager* p);
    bool isDestroyed() const;

    virtual void update(float scroll, Level* lvl);
    virtual void update(PlayerSoldier* player, ProjectileManager* pm,
        float scroll, Level* lvl) = 0;

    virtual void onDeath();
    virtual void updateBoundingBox();
};



class FlyingTara : public EnemyVehicle {
private:
    int taraState;       
    float flySpeed;
    bool bombDropped;
    Animation flyAnim;
    float crashVX;
    float crashVY;
    Clock grenadeTimer;
    Clock deathClock;
    float deathDuration;

public:
    FlyingTara(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~FlyingTara();

    void setFlyDirection(int dir);

    virtual void update(PlayerSoldier* player, ProjectileManager* pm,
        float scroll, Level* lvl);
    virtual void onDeath();
    virtual void updateBoundingBox();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};



class Submarine : public EnemyVehicle {
private:
    int subState;       
    float swimSpeed;
    float patrolLeftX;  
    float patrolRightX; 
    bool bombFired;
    float sinkVY;
    Animation swimAnim;
    Clock bombCooldown;
    Clock deathClock;
    float deathDuration;

public:
    Submarine(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Submarine();

    void setPatrolBounds(float leftX, float rightX);
    void setSwimDirection(int dir);

    virtual void update(PlayerSoldier* player, ProjectileManager* pm,
        float scroll, Level* lvl);
    virtual void onDeath();
    virtual void updateBoundingBox();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};