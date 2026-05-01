#pragma once
#include "Projectile.h"
#include "DamagableEntity.h"

class Level;

class ProjectileManager {
public:
    static const int MAX_PROJ = 64;

private:
    Projectile*     slots[MAX_PROJ];
    int             activeCount;
    TextureManager* texMgr;
    AudioManager*   audMgr;

public:
    ProjectileManager(TextureManager* t, AudioManager* a);
    ~ProjectileManager();

    // ------------------------------------------------------------------
    // WHERE TO SPAWN — call calcBarrelTip() to get the origin,
    // then pass it to spawnStraight / spawnExplosive.
    //
    // Example in PlayerSoldier::shoot():
    //
    //   sf::Vector2f origin = ProjectileManager::calcBarrelTip(
    //       this->position,      // player world position
    //       this->direction,     // DIR_LEFT or DIR_RIGHT
    //       32.f,                // sprite width in pixels (unscaled)
    //       20.f                 // barrel height from top of sprite
    //   );
    //   pm->spawnStraight(origin, this->direction,
    //                     this->aimController.getAngle(), 3, false);
    // ------------------------------------------------------------------
    static sf::Vector2f calcBarrelTip(sf::Vector2f entityPos,
                                       int dir,
                                       float spriteWidth,
                                       float barrelOffsetY);

    void spawnStraight (sf::Vector2f origin, int dir, float angle,
                        int dmg, bool fromEnemy = false);

    void spawnExplosive(sf::Vector2f origin, int dir, float angle,
                        int dmg, int blastRadius, bool fromEnemy = false);

    void update(float scroll, Level* lvl);
    void draw  (RenderWindow& window, float scroll);

    // Pass ANY DamagableEntity** array — enemies, player, blocks
    int  checkEntityCollisions(DamagableEntity** targets, int targetCount);

    int  getActiveCount() const { return this->activeCount; }
    void clearAll();

private:
    void removeAt(int i);

    static void angleToVelocity(float angle, int dir, float speed,
                                 float& outVX, float& outVY);
};