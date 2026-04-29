#pragma once
#include "Entity.h"

// Forward declarations
class ProjectileManager;

// WHY: Weapon is the abstract base class for all weapons
// Provides common properties: ammo, fireRate, damage, type
class Weapon {
protected:
    int ammo;
    float fireRate;
    int damage;
    int type;  // WeaponType constant

public:
    Weapon(int weaponType, int dmg, float rate, int amm);
    virtual ~Weapon();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) = 0;
    virtual void update() = 0;
    
    bool hasAmmo() const;
    void addAmmo(int amount);
    int getAmmo() const;
    int getDamage() const;
    float getFireRate() const;
    int getType() const;
};

// WHY: ProjectileWeapon extends Weapon for weapons that fire projectiles
// Used by Pistol, HMG, RocketLauncher
class ProjectileWeapon : public Weapon {
protected:
    int projectileClass;  // ProjectileClass constant

public:
    ProjectileWeapon(int weaponType, int dmg, float rate, int amm, int projClass);
    virtual ~ProjectileWeapon();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override = 0;
};

// ========== Pistol : ProjectileWeapon ==========
// Damage:3 FireRate:4/s Infinite ammo STRAIGHT
class Pistol : public ProjectileWeapon {
private:
    bool infinite;

public:
    Pistol();
    virtual ~Pistol();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override;
    virtual void update() override;
};

// ========== HeavyMachineGun : ProjectileWeapon ==========
// Damage:3 FireRate:8/s Hold to fire STRAIGHT
class HeavyMachineGun : public ProjectileWeapon {
public:
    HeavyMachineGun();
    virtual ~HeavyMachineGun();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override;
    virtual void update() override;
};

// ========== RocketLauncher : ProjectileWeapon ==========
// Damage:5 in 3-block blast 2s reload BALLISTIC/EXPLOSIVE
class RocketLauncher : public ProjectileWeapon {
private:
    Clock reloadTimer;
    float reloadTime;

public:
    RocketLauncher();
    virtual ~RocketLauncher();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override;
    virtual void update() override;
};

// ========== FlameShot : Weapon (NOT ProjectileWeapon) ==========
// Damage:2 HP/s Instantly kills MummyWarrior Stream via collision area
class FlameShot : public Weapon {
private:
    int streamLength;

public:
    FlameShot();
    virtual ~FlameShot();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override;
    virtual void update() override;
};

// ========== LaserGun : Weapon (NOT ProjectileWeapon) ==========
// Beam to screen edge Instant kill Ray-cast
class LaserGun : public Weapon {
public:
    LaserGun();
    virtual ~LaserGun();

    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) override;
    virtual void update() override;
};
