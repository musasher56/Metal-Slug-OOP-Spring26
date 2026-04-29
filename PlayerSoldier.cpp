#include "PlayerSoldier.h"
#include <iostream>

// ========== PlayerSoldier Implementation ==========

PlayerSoldier::PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Soldier(texMgr, audMgr)
    , currentWeapon(nullptr)
    , pistol(nullptr)
    , currentGrenade(nullptr)
    , grenadeCount(0)
    , inVehicle(false)
    , currentVehicle(nullptr)
    , inventorySize(0)
    , isFat(false)
    , fatGravRadius(0.f)
    , aimController()  // FIX #2: Direct member initialization
{
    // Initialize inventory slots to nullptr
    for (int i = 0; i < 3; ++i) {
        this->inventory[i] = nullptr;
    }
}

PlayerSoldier::~PlayerSoldier() {
    // WHY: PlayerSoldier owns weapons in inventory and currentWeapon
    if (this->currentWeapon != nullptr) {
        delete this->currentWeapon;
        this->currentWeapon = nullptr;
    }
    if (this->pistol != nullptr) {
        delete this->pistol;
        this->pistol = nullptr;
    }
    if (this->currentGrenade != nullptr) {
        delete this->currentGrenade;
        this->currentGrenade = nullptr;
    }
    for (int i = 0; i < this->inventorySize; ++i) {
        if (this->inventory[i] != nullptr) {
            delete this->inventory[i];
            this->inventory[i] = nullptr;
        }
    }
    // Vehicle is not owned by PlayerSoldier, just ejected
}

void PlayerSoldier::switchWeapon(Weapon* w) {
    if (w == nullptr) return;
    
    // Store current weapon back to inventory if valid
    if (this->currentWeapon != nullptr && this->inventorySize < 3) {
        this->inventory[this->inventorySize++] = this->currentWeapon;
    }
    
    this->currentWeapon = w;
}

void PlayerSoldier::throwGrenade() {
    if (this->grenadeCount > 0 && this->currentGrenade != nullptr) {
        this->grenadeCount--;
        // Grenade throwing logic - actual implementation needs ProjectileManager
        // this->currentGrenade->throw(this->position, target);
    }
}

void PlayerSoldier::shoot() {
    // FIX #2: Use aimController.getAngle() for constrained angle
    float angle = this->aimController.getAngle();
    
    if (this->currentWeapon != nullptr && this->currentWeapon->hasAmmo()) {
        // Actual firing requires ProjectileManager - stubbed for now
        // this->currentWeapon->fire(this->position, this->direction, angle, pm);
    }
}

void PlayerSoldier::enterVehicle(Vehicle* v) {
    if (v == nullptr || this->inVehicle) return;
    
    this->currentVehicle = v;
    this->inVehicle = true;
    v->setDriver(this);
}

void PlayerSoldier::exitVehicle() {
    if (!this->inVehicle || this->currentVehicle == nullptr) return;
    
    this->currentVehicle->ejectDriver();
    this->currentVehicle = nullptr;
    this->inVehicle = false;
}

void PlayerSoldier::saveData(std::ofstream& out) {
    if (!out.is_open()) return;
    
    out.write(reinterpret_cast<const char*>(&this->lives), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->currentHP), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->grenadeCount), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->inVehicle), sizeof(bool));
    // Position, velocity, etc.
    out.write(reinterpret_cast<const char*>(&this->position.x), sizeof(float));
    out.write(reinterpret_cast<const char*>(&this->position.y), sizeof(float));
}

void PlayerSoldier::loadData(std::ifstream& in) {
    if (!in.is_open()) return;
    
    in.read(reinterpret_cast<char*>(&this->lives), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->currentHP), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->grenadeCount), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->inVehicle), sizeof(bool));
    in.read(reinterpret_cast<char*>(&this->position.x), sizeof(float));
    in.read(reinterpret_cast<char*>(&this->position.y), sizeof(float));
}

void PlayerSoldier::applyFannumTax(ProjectileManager* pm) {
    // WHY: Fannum Tax lives HERE only on PlayerSoldier, NOT on Soldier
    // This is a special ability that affects projectiles
    if (pm == nullptr || !this->isFat) return;
    
    // Apply fat gravity radius effect to projectiles
    // pm->applyFannumTax(this);  // Stub - requires ProjectileManager
}

void PlayerSoldier::onDeath() {
    // WHY: Handle player death - lose life, respawn or game over
    this->lives--;
    if (this->lives > 0) {
        this->respawn();
    } else {
        // Trigger game over state via GameStateManager
    }
}

void PlayerSoldier::updateBoundingBox() {
    // WHY: Update bounding box based on sprite dimensions
    // Actual implementation depends on sprite size
    this->boundingBox = IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        32,  // Default width
        48   // Default height
    );
}

// ========== Marco Implementation ==========

Marco::Marco(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , fireRateMultiplier(1.25f)
    , dualFireActive(false)
{}

Marco::~Marco() {}

void Marco::updateSprite() {
    // WHY: Marco-specific sprite updates
    // Different animation frames based on state
}

void Marco::activatePowerUp() {
    // WHY: Dual-direction fire for 10 seconds
    this->dualFireActive = true;
    this->dualFireTimer.restart();
}

void Marco::handleInput() {
    // WHY: Marco handles keyboard/mouse input for movement and actions
    // Actual implementation in PlayState event handling
}

void Marco::meleeAttack() {
    // WHY: Marco's melee pierces shields
    // Special melee logic for shield-piercing
    Soldier::meleeAttack();
}

// ========== Tarma Implementation ==========

Tarma::Tarma(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , vehicleFireRateBonus(0.25f)
    , vehicleDurabilityBonus(0.20f)
    , immunityActive(false)
{}

Tarma::~Tarma() {}

void Tarma::updateSprite() {
    // WHY: Tarma-specific sprite updates
}

void Tarma::activatePowerUp() {
    // WHY: 20 seconds immunity
    this->immunityActive = true;
    this->immunityTimer.restart();
}

void Tarma::handleInput() {
    // WHY: Tarma handles keyboard/mouse input
}

bool Tarma::hasVehicleSurvival() const {
    // WHY: Tarma survives vehicle destruction
    return true;
}

void Tarma::onVehicleDestroyed() {
    // WHY: Tarma's special ability - survives when vehicle is destroyed
    // Eject safely instead of taking damage
    this->exitVehicle();
}

// ========== Eri Implementation ==========

Eri::Eri(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , blastRadiusMultiplier(1.50f)
    , doubleGrenadeActive(false)
{}

Eri::~Eri() {}

void Eri::updateSprite() {
    // WHY: Eri-specific sprite updates
}

void Eri::activatePowerUp() {
    // WHY: 2 grenades for cost of 1 for 10 seconds
    this->doubleGrenadeActive = true;
    this->doubleGrenadeTimer.restart();
}

void Eri::handleInput() {
    // WHY: Eri handles keyboard/mouse input
}

void Eri::throwGrenade() {
    // WHY: Eri throws FireBombGrenade with double throw capability
    if (this->doubleGrenadeActive && this->grenadeCount >= 2) {
        this->grenadeCount -= 2;  // Throws 2 but costs 1
        // Throw two grenades
    } else if (this->grenadeCount > 0) {
        this->grenadeCount--;
        // Throw single grenade
    }
}

// ========== Fio Implementation ==========

Fio::Fio(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , ammoBonusMultiplier(1.50f)
    , fireRateMultiplier(1.10f)
    , superchargedActive(false)
{}

Fio::~Fio() {}

void Fio::updateSprite() {
    // WHY: Fio-specific sprite updates
}

void Fio::activatePowerUp() {
    // WHY: Fire rate ×2.0 for 10 seconds (SUPERCHARGED)
    this->superchargedActive = true;
    this->superchargedTimer.restart();
}

void Fio::handleInput() {
    // WHY: Fio handles keyboard/mouse input
}

void Fio::pickUpWeapon() {
    // WHY: Fio gets +50% ammo bonus on pickup
    // Override weapon pickup to add bonus ammo
}
