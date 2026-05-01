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
    Pistol*           pistol;          // always available, infinite ammo
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

    // WHY store pm here instead of passing to shoot()?
    //   handleInput() is pure virtual with no parameters — every input
    //   path that triggers shoot() would need pm threaded through it.
    //   Storing it as a non-owning pointer set once at play-start is cleaner.
    //   PlayerSoldier does NOT own this pointer — never delete it here.
    ProjectileManager* pm;

public:
    PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayerSoldier();

    // Call this once from PlayState after creating both player and pm
    void setProjectileManager(ProjectileManager* manager);

    // Returns current aim angle (0=horizontal, 90=up) — used by debug overlay
    float getAimAngle() const { return this->aimController.getAngle(); }

    void switchWeapon(Weapon* w);
    void throwGrenade();

    // shoot() now fully implemented — uses pm, aimController, currentWeapon
    void shoot();

    void enterVehicle(Vehicle* v);
    void exitVehicle();
    void saveData(std::ofstream& out);
    void loadData(std::ifstream& in);

    // Call every frame from CharacterManager/PlayState with current mouse pos
    // WHY separate from shoot()?  Aim updates every frame; shoot only on keypress.
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