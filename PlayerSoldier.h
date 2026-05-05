#pragma once
#include "Soldier.h"
#include "AimController.h"
#include "Weapon.h"
#include "Grenade.h"
#include "Vehicle.h"
#include "ProjectileManager.h"

// WHY: PlayerSoldier is the abstract base class for all player characters
class PlayerSoldier : public Soldier {
protected:
    Weapon*           currentWeapon;
    Pistol*           pistol;
    Grenade*          currentGrenade;
    int               grenadeCount;
    bool              inVehicle;
    Vehicle*          currentVehicle;
    Weapon*           inventory[3];
    int               inventorySize;
    bool              isFat;
    float             fatGravRadius;
    Clock             stateTimer;
    AimController     aimController;

    ProjectileManager* pm;

public:
    PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayerSoldier();

    void setProjectileManager(ProjectileManager* manager);

    float getAimAngle() const { return this->aimController.getAngle(); }

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
    virtual void updateSprite()    = 0;
    virtual void activatePowerUp() = 0;
    void onDeath();
    void updateBoundingBox();
    virtual void handleInput() = 0;
};

// ========== Marco ==========
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

// ========== Tarma ==========
class Tarma : public PlayerSoldier {
private:
    float vehicleFireRateBonus;
    float vehicleDurabilityBonus;
    bool  immunityActive;
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

// ========== Eri ==========
class Eri : public PlayerSoldier {
private:
    float blastRadiusMultiplier;
    bool  doubleGrenadeActive;
    Clock doubleGrenadeTimer;
public:
    Eri(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Eri();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    virtual void throwGrenade();
};

// ========== Fio ==========
class Fio : public PlayerSoldier {
private:
    float ammoBonusMultiplier;
    float fireRateMultiplier;
    bool  superchargedActive;
    Clock superchargedTimer;
public:
    Fio(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Fio();
    virtual void updateSprite();
    virtual void activatePowerUp();
    virtual void handleInput();
    void pickUpWeapon();
};
