// ============================================================
// Weapon.h  — add Clock fireTimer to Weapon base
// ============================================================
// ONLY CHANGE from previous version:
//   Added  Clock fireTimer;  to the protected section of Weapon.
//   WHY here instead of each subclass?
//   Every weapon that fires projectiles needs a cadence timer.
//   Putting it in the base avoids duplicating it in Pistol, HMG,
//   RocketLauncher, etc.  FlameShot and LaserGun can reuse it for
//   their own hold/cooldown logic later.
// ============================================================
#pragma once
#include "Entity.h"

class ProjectileManager;

class Weapon {
protected:
    int   ammo;
    float fireRate;
    int   damage;
    int   type;
    Clock fireTimer;   // <-- FIX: was missing, caused "no member" errors

public:
    Weapon(int weaponType, int dmg, float rate, int amm);
    virtual ~Weapon();

    virtual void fire(sf::Vector2f origin, int dir, float angle,
                      ProjectileManager* pm) = 0;
    virtual void update() = 0;

    bool  hasAmmo()     const;
    void  addAmmo(int amount);
    int   getAmmo()     const;
    int   getDamage()   const;
    float getFireRate() const;
    int   getType()     const;
};

class ProjectileWeapon : public Weapon {
protected:
    int projectileClass;
public:
    ProjectileWeapon(int weaponType, int dmg, float rate, int amm, int projClass);
    virtual ~ProjectileWeapon();
    virtual void fire(sf::Vector2f origin, int dir, float angle,
                      ProjectileManager* pm) = 0;
};

// Pistol — Damage:3  FireRate:4/s  Infinite ammo  STRAIGHT
class Pistol : public ProjectileWeapon {
private:
    bool infinite;
public:
    Pistol();
    virtual ~Pistol();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};

// HeavyMachineGun — Damage:3  FireRate:8/s  100 ammo  STRAIGHT
class HeavyMachineGun : public ProjectileWeapon {
public:
    HeavyMachineGun();
    virtual ~HeavyMachineGun();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};

// RocketLauncher — Damage:5  2s reload  3-block blast  EXPLOSIVE
class RocketLauncher : public ProjectileWeapon {
private:
    Clock reloadTimer;
    float reloadTime;
public:
    RocketLauncher();
    virtual ~RocketLauncher();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};

// FlameShot — stream zone, NOT a projectile weapon
class FlameShot : public Weapon {
private:
    int streamLength;
public:
    FlameShot();
    virtual ~FlameShot();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};

// LaserGun — instant ray-cast, NOT a projectile weapon
class LaserGun : public Weapon {
public:
    LaserGun();
    virtual ~LaserGun();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};