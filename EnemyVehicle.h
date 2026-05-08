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