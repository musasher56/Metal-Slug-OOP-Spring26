#pragma once
#include "Soldier.h"
#include "AimController.h"
#include "Weapon.h"
#include "Grenade.h"
#include "Vehicle.h"
#include "ProjectileManager.h"


class PlayerSoldier : public Soldier {
protected:
    Weapon* currentWeapon;
    Pistol* pistol;
    Grenade* currentGrenade;
    int      grenadeCount;
    bool     inVehicle;
    Vehicle* currentVehicle;
    Weapon* inventory[3];
    int      inventorySize;
    bool     isFat;
    float    fatGravRadius;
    Clock    stateTimer;
    AimController aimController;
    int      enemyBulletHits;

    ProjectileManager* pm;

    // ── Dev weapon cycle (Q key) ───────────────────────────────────────────
    // Pre-created instances of every weapon type so Q can switch instantly
    // without allocating on each keypress.  The pool is created in the
    // PlayerSoldier constructor and destroyed in its destructor.
    //
    // Slot layout:
    //   [0] Pistol          — bright yellow,  infinite ammo
    //   [1] HeavyMachineGun — bright yellow,  100 rounds,  8/sec
    //   [2] RocketLauncher  — orange capsule, 10 rockets,  2s reload
    //   [3] FlameShot       — orange→red fade, 50 fuel,    stream
    //   [4] LaserGun        — cyan beam,       20 charges, 2s cooldown
    //
    // WHY store as a flat pool instead of using inventory[3]?
    //   inventory[] is the gameplay pickup system (supply crates).
    //   The dev pool is a separate concern — testing only.  Mixing them
    //   would corrupt the pickup ammo values and complicate destructor logic.
    Weapon* devWeaponPool[5];
    int     devWeaponIdx;   // index of the currently active weapon in the pool

    // Edge-detect flag: prevents Q held-down from cycling every frame.
    // Cycle fires once on the PRESS, not on every tick it's held.
    bool qWasPressed;

    // Cycles to the next weapon in devWeaponPool and sets currentWeapon.
    // Called by every character's handleInput() on Q press.
    void cycleWeapon();

public:
    PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayerSoldier();

    void setProjectileManager(ProjectileManager* manager);

    float getAimAngle()        const { return this->aimController.getAngle(); }
    int   getEnemyBulletHits() const { return this->enemyBulletHits; }

    // Returns the display name of the currently active weapon type.
    // Useful for HUD or debug overlay — no std::string: returns a string literal.
    const char* getCurrentWeaponName() const;

    // Heal player to full HP and optionally increase max HP (used after boss defeat)
    void healFullAndIncreaseHP(int extraHP = 1);

    void switchWeapon(Weapon* w);
    void throwGrenade();
    void shoot();

    void enterVehicle(Vehicle* v);
    void exitVehicle();
    void saveData(std::ofstream& out);
    void loadData(std::ifstream& in);

    void updateAim(sf::Vector2f mousePos);

protected:
    void applyFannumTax(ProjectileManager* manager);

public:
    virtual void updateSprite() = 0;
    virtual void activatePowerUp() = 0;
    void onDeath();
    void updateBoundingBox();
    virtual void handleInput() = 0;
    virtual void takeDamage(int amount);

    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};


class Marco : public PlayerSoldier {
private:
    float fireRateMultiplier;
    bool  dualFireActive;
    Clock dualFireTimer;
public:
    Marco(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Marco();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void meleeAttack();
};


class Tarma : public PlayerSoldier {
private:
    float vehicleFireRateBonus;
    float vehicleDurabilityBonus;
    bool  immunityActive;
    Clock immunityTimer;
    Animation walkAnim;
public:
    Tarma(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Tarma();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    bool hasVehicleSurvival() const;
    void onVehicleDestroyed();
};


class Eri : public PlayerSoldier {
private:
    float blastRadiusMultiplier;
    bool  doubleGrenadeActive;
    Clock doubleGrenadeTimer;
    Animation walkAnim;
public:
    Eri(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Eri();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void meleeAttack();
    virtual void throwGrenade();
};


class Fio : public PlayerSoldier {
private:
    float ammoBonusMultiplier;
    float fireRateMultiplier;
    bool  superchargedActive;
    Clock superchargedTimer;
    Animation walkAnim;
public:
    Fio(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Fio();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void switchWeapon(Weapon* w);
};