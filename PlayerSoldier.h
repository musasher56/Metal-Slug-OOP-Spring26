#pragma once
#include "Soldier.h"
#include "AimController.h"
#include "Weapon.h"
#include "Grenade.h"
#include "Vehicle.h"
#include "ProjectileManager.h"

// Forward declarations
class ProjectileManager;

// WHY: PlayerSoldier is the abstract base class for all player characters
// Extends Soldier with weapon handling, vehicle support, inventory, and aim controller
class PlayerSoldier : public Soldier {
protected:
    Weapon* currentWeapon;
    Pistol* pistol;
    Grenade* currentGrenade;
    int grenadeCount;
    bool inVehicle;
    Vehicle* currentVehicle;
    Weapon* inventory[3];
    int inventorySize;
    bool isFat;
    float fatGravRadius;
    Clock stateTimer;
    AimController aimController;  // FIX #2: Direct member, not pointer

public:
    PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayerSoldier();

    void switchWeapon(Weapon* w);
    void throwGrenade();
    void shoot();  // Uses aimController.getAngle() for constrained angle
    void enterVehicle(Vehicle* v);
    void exitVehicle();
    void saveData(std::ofstream& out);
    void loadData(std::ifstream& in);

protected:
    virtual void handleInput() = 0;  // Pure virtual - concrete chars implement
    void applyFannumTax(ProjectileManager* pm);  // Only on PlayerSoldier, NOT Soldier

public:
    virtual void updateSprite() = 0;     // Pure virtual
    virtual void activatePowerUp() = 0;  // Pure virtual
    void onDeath() override;
    void updateBoundingBox() override;
};

// ========== Marco : PlayerSoldier ==========
// Buffs: fireRate×1.25, melee pierces shields | PowerUp: dual-direction fire 10s
class Marco : public PlayerSoldier {
private:
    float fireRateMultiplier;
    bool dualFireActive;
    Clock dualFireTimer;

public:
    Marco(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Marco();

    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void meleeAttack();
};

// ========== Tarma : PlayerSoldier ==========
// Buffs: vehicle fireRate×1.25, durability×1.20 | PowerUp: 20s immunity
class Tarma : public PlayerSoldier {
private:
    float vehicleFireRateBonus;
    float vehicleDurabilityBonus;
    bool immunityActive;
    Clock immunityTimer;

public:
    Tarma(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Tarma();

    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    
    bool hasVehicleSurvival() const;
    void onVehicleDestroyed();
};

// ========== Eri : PlayerSoldier ==========
// Buffs: grenadeCount×2, blast radius×1.50 | PowerUp: 2 grenades for cost of 1 for 10s
class Eri : public PlayerSoldier {
private:
    float blastRadiusMultiplier;
    bool doubleGrenadeActive;
    Clock doubleGrenadeTimer;

public:
    Eri(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Eri();

    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void throwGrenade();
};

// ========== Fio : PlayerSoldier ==========
// Buffs: +50% ammo on pickup, fireRate×1.10 | PowerUp: fireRate×2.0 for 10s (SUPERCHARGED)
class Fio : public PlayerSoldier {
private:
    float ammoBonusMultiplier;
    float fireRateMultiplier;
    bool superchargedActive;
    Clock superchargedTimer;

public:
    Fio(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Fio();

    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    void pickUpWeapon();
};
