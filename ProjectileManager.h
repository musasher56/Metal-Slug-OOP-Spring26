#pragma once
#include "Projectile.h"
#include "DamagableEntity.h"

class Level;

// ─────────────────────────────────────────────────────────────────────────────
// ProjectileManager
//
// Manages the lifetime of every in-flight projectile via a fixed-size slot
// array (Object Pool pattern).  Slots are never reallocated; unused entries
// hold nullptr.  activeCount is the compact active count — swapping a dead slot
// with the tail entry (removeAt) keeps iteration dense and avoids fragmentation.
//
// draw() is deliberately thin: it calls slots[i]->draw(...) and the vtable
// dispatches the correct coloured placeholder per subclass (StraightProjectile →
// yellow, FlameParticle → orange/red, LaserBeam → cyan, ExplosiveProjectile →
// orange or grey).  There are NO if-else type-switches for game logic in this
// class — all behaviour lives inside the Projectile hierarchy.
// ─────────────────────────────────────────────────────────────────────────────
class ProjectileManager {
public:
    static const int MAX_PROJ   = 64;   // hard cap on simultaneous projectiles
    static const int MAX_BLASTS = 16;   // explosion animation pool

private:
    Projectile*     slots[MAX_PROJ];
    int             activeCount;
    TextureManager* texMgr;
    AudioManager*   audMgr;

    // Blast effect pool — explosion animation sprites, independent of Projectile
    // lifetime.  They continue rendering after the ExplosiveProjectile deactivates.
    struct BlastEffect {
        Animation anim;
        float     x;
        float     y;
        bool      active;
    };
    BlastEffect blasts[MAX_BLASTS];
    int         blastCount;

public:
    ProjectileManager(TextureManager* t, AudioManager* a);
    ~ProjectileManager();

    // ── Spawn helpers ─────────────────────────────────────────────────────────
    // Each helper creates the appropriate Projectile subclass, sets its fields
    // via the friend relationship, and inserts it into the slot array.
    // They are separate functions (not one overloaded spawn()) to keep call
    // sites readable and to express intent at the call site.

    // Pistol / HMG — yellow bullet, constant velocity
    void spawnStraight(sf::Vector2f origin, int dir, float angle,
                       int dmg, bool fromEnemy = false);

    // RocketLauncher — orange capsule, ballistic arc, blast radius
    void spawnExplosive(sf::Vector2f origin, int dir, float angle,
                        int dmg, int blastRadius, bool fromEnemy = false);

    // Enemy-thrown bomb — dark grey sphere, ballistic, PROJ_BOMB tag
    void spawnBomb(sf::Vector2f origin, int dir, float angle,
                   int dmg, int blastRadius, bool fromEnemy = false,
                   float speed = 8.f);

    // FlameShot — spawns one FlameParticle per call.
    // The stream effect comes from high fireRate producing many overlapping particles.
    void spawnFlame(sf::Vector2f origin, int dir, float angle,
                    int dmg, bool fromEnemy = false);

    // LaserGun — one stationary LaserBeam; its wide AABB hits all targets in path.
    void spawnLaser(sf::Vector2f origin, int dir, int dmg, bool fromEnemy = false);

    // Explosion visual at world position (called by collision handlers)
    void spawnBlast(float x, float y);

    // ── Utility ───────────────────────────────────────────────────────────────
    static sf::Vector2f calcBarrelTip(sf::Vector2f entityPos,
                                      int          dir,
                                      float        spriteWidth,
                                      float        barrelOffsetY);

    // ── Per-frame pipeline ────────────────────────────────────────────────────
    void update(float scroll, Level* lvl);
    void postEntityUpdate(float scrollX, float scrollY, Level* lvl);

    // Pure polymorphic dispatch — no type-checking in the loop body.
    void draw(RenderWindow& window, float scrollX, float scrollY);

    // ── Collision queries ─────────────────────────────────────────────────────
    int  checkEntityCollisions(DamagableEntity** targets, int targetCount);
    int  checkPlayerBulletHits(DamagableEntity** targets, int targetCount);
    bool checkEnemyBulletHitPlayer(DamagableEntity* player);

    // ── Accessors ─────────────────────────────────────────────────────────────
    int          getActiveCount() const { return this->activeCount; }
    Projectile** getSlots()             { return this->slots;       }
    void         clearAll();

private:
    // Compact removal: swap dead slot with tail entry, decrement activeCount.
    // O(1), preserves dense packing — critical for tight update() loops.
    void removeAt(int i);

    // Converts (angle, direction, speed) → (vx, vy).
    // angle = 0 means horizontal; positive angle tilts upward.
    // dir multiplies the X component sign so left-facing fire reverses correctly.
    static void angleToVelocity(float angle, int dir, float speed,
                                 float& outVX, float& outVY);
};