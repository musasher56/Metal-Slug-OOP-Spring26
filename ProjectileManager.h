#pragma once
#include "Projectile.h"
#include "DamagableEntity.h"

class Level;

class ProjectileManager {
public:
    static const int MAX_PROJ = 64;

private:
    Projectile* slots[MAX_PROJ];
    int             activeCount;
    TextureManager* texMgr;
    AudioManager* audMgr;

public:
    ProjectileManager(TextureManager* t, AudioManager* a);
    ~ProjectileManager();

    static sf::Vector2f calcBarrelTip(sf::Vector2f entityPos,
        int dir,
        float spriteWidth,
        float barrelOffsetY);

    void spawnStraight(sf::Vector2f origin, int dir, float angle,
        int dmg, bool fromEnemy = false);

    void spawnExplosive(sf::Vector2f origin, int dir, float angle,
        int dmg, int blastRadius, bool fromEnemy = false);

    // Phase 1: Move all projectiles (no tile collision yet)
    void update(float scroll, Level* lvl);

    // Phase 3: Tile collision + bounds + cleanup
    // Call AFTER checkEntityCollisions so destroyed blocks
    // have cleared their grid cells before tile check runs
    void postEntityUpdate(float scroll, Level* lvl);

    void draw(RenderWindow& window, float scroll);

    int  checkEntityCollisions(DamagableEntity** targets, int targetCount);

    int  getActiveCount() const { return this->activeCount; }
    void clearAll();

private:
    void removeAt(int i);

    static void angleToVelocity(float angle, int dir, float speed,
        float& outVX, float& outVY);
};