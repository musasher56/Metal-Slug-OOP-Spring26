#include "Weapon.h"

// ========== Weapon Implementation ==========

Weapon::Weapon(int weaponType, int dmg, float rate, int amm)
    : ammo(amm)
    , fireRate(rate)
    , damage(dmg)
    , type(weaponType)
{}

Weapon::~Weapon() {}

bool Weapon::hasAmmo() const {
    return this->ammo > 0 || this->ammo == -1;  // -1 means infinite
}

void Weapon::addAmmo(int amount) {
    if (amount < 0) return;
    if (this->ammo != -1) {  // Don't add to infinite ammo
        this->ammo += amount;
    }
}

int Weapon::getAmmo() const {
    return this->ammo;
}

int Weapon::getDamage() const {
    return this->damage;
}

float Weapon::getFireRate() const {
    return this->fireRate;
}

int Weapon::getType() const {
    return this->type;
}

// ========== ProjectileWeapon Implementation ==========

ProjectileWeapon::ProjectileWeapon(int weaponType, int dmg, float rate, int amm, int projClass)
    : Weapon(weaponType, dmg, rate, amm)
    , projectileClass(projClass)
{}

ProjectileWeapon::~ProjectileWeapon() {}

// ========== Pistol Implementation ==========

Pistol::Pistol()
    : ProjectileWeapon(WEAPON_PISTOL, 3, 4.f, -1, PROJ_STRAIGHT)
    , infinite(true)
{}

Pistol::~Pistol() {}

void Pistol::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!this->hasAmmo()) return;
    
    // WHY: Pistol fires straight projectiles
    // Infinite ammo so no decrement needed
    if (pm != nullptr) {
        // pm->addStraightProjectile(origin, dir, angle, this->damage);
    }
}

void Pistol::update() {
    // WHY: No special update logic for pistol
}

// ========== HeavyMachineGun Implementation ==========

HeavyMachineGun::HeavyMachineGun()
    : ProjectileWeapon(WEAPON_HMG, 3, 8.f, 100, PROJ_STRAIGHT)
{}

HeavyMachineGun::~HeavyMachineGun() {}

void HeavyMachineGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!this->hasAmmo()) return;
    
    this->ammo--;
    if (pm != nullptr) {
        // pm->addStraightProjectile(origin, dir, angle, this->damage);
    }
}

void HeavyMachineGun::update() {
    // WHY: No special update logic for HMG
}

// ========== RocketLauncher Implementation ==========

RocketLauncher::RocketLauncher()
    : ProjectileWeapon(WEAPON_ROCKET_LAUNCHER, 5, 0.5f, 10, PROJ_EXPLOSIVE)
    , reloadTime(2.0f)
{}

RocketLauncher::~RocketLauncher() {}

void RocketLauncher::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!this->hasAmmo()) return;
    
    if (this->reloadTimer.getElapsedTime().asSeconds() < this->reloadTime) {
        return;  // Still reloading
    }
    
    this->ammo--;
    this->reloadTimer.restart();
    
    if (pm != nullptr) {
        // pm->addExplosiveProjectile(origin, dir, angle, this->damage, 3);  // 3-block blast radius
    }
}

void RocketLauncher::update() {
    // WHY: Reload timer handled in fire()
}

// ========== FlameShot Implementation ==========

FlameShot::FlameShot()
    : Weapon(WEAPON_FLAME_SHOT, 2, 1.f, 50)
    , streamLength(5)  // 5 blocks
{}

FlameShot::~FlameShot() {}

void FlameShot::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!this->hasAmmo()) return;
    
    this->ammo--;
    
    // WHY: FlameShot creates a stream/collision area, not a projectile
    // Actual implementation needs collision detection with enemies in front
    (void)origin;
    (void)dir;
    (void)angle;
    (void)pm;
}

void FlameShot::update() {
    // WHY: Stream duration handled externally
}

// ========== LaserGun Implementation ==========

LaserGun::LaserGun()
    : Weapon(WEAPON_LASER_GUN, 999, 0.5f, 20)  // Instant kill = high damage
{}

LaserGun::~LaserGun() {}

void LaserGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!this->hasAmmo()) return;
    
    this->ammo--;
    
    // WHY: LaserGun performs ray-cast to screen edge, instant hit
    // Not a projectile - immediate damage calculation
    (void)origin;
    (void)dir;
    (void)angle;
    (void)pm;
}

void LaserGun::update() {
    // WHY: No special update logic for laser
}
