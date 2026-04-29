#pragma once
#include "DamagableEntity.h"

class ProjectileManager;
class PlayerSoldier;

// WHY: Vehicle is abstract base for all vehicles (ground, aerial, aquatic)
// Uses virtual inheritance from DamagableEntity to resolve diamond problem in AmphibiousSlug
class Vehicle : virtual public DamagableEntity {
protected:
    int maxHealth;
    float durability;
    PlayerSoldier* driver;
    float velocityX;
    float velocityY;
    float fireRate;
    int mode;  // VehicleMode: 0=GROUND, 1=AERIAL, 2=AQUATIC

public:
    Vehicle(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Vehicle();

    virtual void shoot(ProjectileManager* pm) = 0;
    bool isDestroyed() const;
    int getMode() const;
    void setDriver(PlayerSoldier* c);
    void ejectDriver();
    void onDeath();
    virtual void updateBoundingBox() = 0;
    
protected:
    void applyTarmaBuffs();
};

// WHY: GroundVehicle adds ground physics
class GroundVehicle : virtual public Vehicle {
protected:
    void applyGroundPhysics(Level* lvl);

public:
    GroundVehicle(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~GroundVehicle();
    
    virtual void shoot(ProjectileManager* pm) = 0;
};

// WHY: AerialVehicle adds aerial physics
class AerialVehicle : virtual public Vehicle {
protected:
    void applyAerialPhysics();

public:
    AerialVehicle(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~AerialVehicle();
    
    virtual void shoot(ProjectileManager* pm) = 0;
};

// WHY: AquaticVehicle adds aquatic physics
class AquaticVehicle : virtual public Vehicle {
protected:
    void applyAquaticPhysics();

public:
    AquaticVehicle(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~AquaticVehicle();
    
    virtual void shoot(ProjectileManager* pm) = 0;
};
