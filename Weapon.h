#pragma once
#include "Entity.h"

class ProjectileManager;
class Weapon {
protected:
    int   ammo;
    float fireRate;
    int   damage;
    int   type;
    Clock fireTimer;
public:
    Weapon(int weaponType, int dmg, float rate, int amm);
    virtual ~Weapon();
    virtual void fire(sf::Vector2f origin, int dir, float angle,ProjectileManager* pm) = 0;
    virtual void update() = 0;

    bool  hasAmmo()     const;
    void  addAmmo(int amount);
    int   getAmmo()     const;
    int   getDamage()   const;
    float getFireRate() const;
    int   getType()     const;
    void  setFireRate(float r);
};

class ProjectileWeapon : public Weapon {
protected:
    int projectileClass;
public:
    ProjectileWeapon(int weaponType, int dmg, float rate, int amm, int projClass);
    virtual ~ProjectileWeapon();
    virtual void fire(sf::Vector2f origin, int dir, float angle,ProjectileManager* pm) = 0;
};


class Pistol : public ProjectileWeapon {
private:
    bool infinite;
public:
    Pistol();
    virtual ~Pistol();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};


class HeavyMachineGun : public ProjectileWeapon {
public:
    HeavyMachineGun();
    virtual ~HeavyMachineGun();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};


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


class FlameShot : public Weapon {
private:
    int streamLength;
public:
    FlameShot();
    virtual ~FlameShot();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};


class LaserGun : public Weapon {
public:
    LaserGun();
    virtual ~LaserGun();
    virtual void fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm);
    virtual void update();
};