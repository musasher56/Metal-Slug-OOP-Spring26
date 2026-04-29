#include "Vehicle.h"
#include "PlayerSoldier.h"

Vehicle::Vehicle(TextureManager* texMgr, AudioManager* audMgr)
    : DamagableEntity(texMgr, audMgr), maxHealth(10), durability(1.0f),
      driver(nullptr), velocityX(0), velocityY(0), fireRate(2.0f), mode(0) {
}

Vehicle::~Vehicle() {
}

bool Vehicle::isDestroyed() const {
    return this->health <= 0;
}

int Vehicle::getMode() const {
    return this->mode;
}

void Vehicle::setDriver(PlayerSoldier* c) {
    this->driver = c;
}

void Vehicle::ejectDriver() {
    this->driver = nullptr;
}

void Vehicle::onDeath() {
    // WHY: Handle vehicle destruction
    if (this->driver != nullptr) {
        this->driver->exitVehicle();
    }
}

void Vehicle::applyTarmaBuffs() {
    // WHY: Apply Tarma's vehicle buffs if driver is Tarma
    // Implemented in Tarma class
}

GroundVehicle::GroundVehicle(TextureManager* texMgr, AudioManager* audMgr)
    : Vehicle(texMgr, audMgr) {
    this->mode = VEHICLE_GROUND;
}

GroundVehicle::~GroundVehicle() {
}

void GroundVehicle::applyGroundPhysics(Level* lvl) {
    // WHY: Apply ground physics (gravity, collision)
    // Stub - implemented in concrete classes
}

AerialVehicle::AerialVehicle(TextureManager* texMgr, AudioManager* audMgr)
    : Vehicle(texMgr, audMgr) {
    this->mode = VEHICLE_AERIAL;
}

AerialVehicle::~AerialVehicle() {
}

void AerialVehicle::applyAerialPhysics() {
    // WHY: Apply aerial physics
    // Stub - implemented in concrete classes
}

AquaticVehicle::AquaticVehicle(TextureManager* texMgr, AudioManager* audMgr)
    : Vehicle(texMgr, audMgr) {
    this->mode = VEHICLE_AQUATIC;
}

AquaticVehicle::~AquaticVehicle() {
}

void AquaticVehicle::applyAquaticPhysics() {
    // WHY: Apply aquatic physics
    // Stub - implemented in concrete classes
}
