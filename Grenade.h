#pragma once
#include "Entity.h"


class EnemyManager;
class CharacterManager;


// ─────────────────────────────────────────────────────────────────────────────
// Grenade — abstract base for all player-thrown explosives
//
// WHY draw() is virtual here:
//   FireBombGrenade has a unique visual (flight + persistent fire pool on
//   impact) that can't be expressed by the base olive-green sphere alone.
//   Making draw() virtual lets FireBombGrenade::draw() override it cleanly
//   through the vtable — no type-checking needed at the call site.
//   This is the same polymorphic rendering principle used in the Projectile
//   hierarchy (each subclass draws itself; the caller just calls draw()).
//
// WHY Grenade is NOT part of the Projectile hierarchy:
//   Grenade objects are owned and updated by PlayerSoldier directly (composition).
//   Projectiles live in ProjectileManager's pool.  Mixing them would blur the
//   ownership boundary and risk double-deletion (P3 penalty: -150).
// ─────────────────────────────────────────────────────────────────────────────
class Grenade {
protected:
    sf::Vector2f position;
    int          blastRadius;
    int          damage;
    sf::Vector2f velocity;
    bool         active;

public:
    Grenade(int radius, int dmg);
    virtual ~Grenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target) = 0;
    virtual void onImpact(EnemyManager* em) = 0;

    void update();

    // Virtual so FireBombGrenade can override to also draw the fire pool.
    // No override keyword per project constraint — implicit virtual dispatch.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    bool         isActive()      const;
    sf::Vector2f getPosition()   const;
    int          getBlastRadius()const;
    int          getDamage()     const;
};



class HandGrenade : public Grenade {
public:
    HandGrenade();
    virtual ~HandGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);
    // Inherits Grenade::draw() — olive-green sphere is correct for HandGrenade.
};



class FireBombGrenade : public Grenade {
private:
    Clock poolDuration;
    bool  poolActive;
    int   poolRadius;
    float poolLifetime;

public:
    FireBombGrenade();
    virtual ~FireBombGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);

    // Overrides base draw() to render the orange-red grenade in flight AND
    // the persistent fire-pool rectangle after impact.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    void spawnFirePool();
};