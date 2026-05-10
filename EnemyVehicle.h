#pragma once
#include "DamagableEntity.h"
#include "Animation.h"

class PlayerSoldier;
class ProjectileManager;
class Level;

// ============================================================
// EnemyVehicle — base class for all enemy vehicles
// Inherits: Entity → DamagableEntity → EnemyVehicle
// ============================================================

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

// ============================================================
// FlyingTara — enemy bomber plane
// HP: 5, hovers across screen, drops bomb on player
// When shot: crashes at angle, spawns blast on ground impact
// ============================================================

class FlyingTara : public EnemyVehicle {
private:
    int taraState;       // 0 = flying, 1 = crashing
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

// ============================================================
// Submarine — aquatic enemy vehicle
// HP: 8, swims back and forth in the water region,
// fires bomb (same as FlyingTara) when player is nearby.
// Sprite: submarine.png (7 frames)
// ============================================================

class Submarine : public EnemyVehicle {
private:
    int subState;       // 0 = swimming, 1 = destroyed/sinking
    float swimSpeed;
    float patrolLeftX;  // left patrol boundary
    float patrolRightX; // right patrol boundary
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