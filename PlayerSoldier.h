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
    Weapon* devWeaponPool[5];
    int     devWeaponIdx;
    bool qWasPressed;
    void cycleWeapon();
    void applyFannumTax(ProjectileManager* manager);
public:
    PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~PlayerSoldier();
    void setProjectileManager(ProjectileManager* manager);
    float getAimAngle()        const { return this->aimController.getAngle(); }
    int   getEnemyBulletHits() const { return this->enemyBulletHits; }
    Weapon* getCurrentWeapon() const { return this->currentWeapon; }
    int     getGrenadeCount() const { return this->grenadeCount; }
    const char* getCurrentWeaponName() const;
    void healFullAndIncreaseHP(int extraHP = 1);
    void switchWeapon(Weapon* w);
    void throwGrenade();
    void shoot();

    void enterVehicle(Vehicle* v);
    void exitVehicle();
    void saveData(std::ofstream& out);
    void loadData(std::ifstream& in);
    void updateAim(sf::Vector2f mousePos);
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