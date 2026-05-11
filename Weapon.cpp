#include "Weapon.h"
#include "ProjectileManager.h"


Weapon::Weapon(int weaponType, int dmg, float rate, int amm)
    : ammo(amm), fireRate(rate), damage(dmg), type(weaponType)
{}

Weapon::~Weapon() {}

bool  Weapon::hasAmmo()     const {
    return this->ammo > 0 || this->ammo == -1;
}
int   Weapon::getAmmo()     const {
    return this->ammo;    
}
int   Weapon::getDamage()   const { 
    
    return this->damage;  
}
float Weapon::getFireRate() const {
    return this->fireRate;
}
int   Weapon::getType()     const { 
    return this->type;   
}

void Weapon::addAmmo(int amount) {
    if (amount < 0) return;
    
    if (this->ammo != -1) this->ammo += amount;
}

void Weapon::setFireRate(float r) {
    
    
    if (r > 0.f) this->fireRate = r;
}





ProjectileWeapon::ProjectileWeapon(int wt, int dmg, float rate, int amm, int pc)
    : Weapon(wt, dmg, rate, amm), projectileClass(pc)
{}

ProjectileWeapon::~ProjectileWeapon() {}









Pistol::Pistol()
    : ProjectileWeapon(WEAPON_PISTOL, 3, 4.f, -1, PROJ_STRAIGHT)
    , infinite(true)
{}

Pistol::~Pistol() {}

void Pistol::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    
    
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    pm->spawnStraight(origin, dir, angle, this->damage, false);
    this->fireTimer.restart();
}

void Pistol::update() {
    
    
    
}












HeavyMachineGun::HeavyMachineGun()
    : ProjectileWeapon(WEAPON_HMG, 3, 8.f, 100, PROJ_STRAIGHT)
{}

HeavyMachineGun::~HeavyMachineGun() {}

void HeavyMachineGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    
    
    this->ammo--;
    pm->spawnStraight(origin, dir, angle, this->damage, false);
    this->fireTimer.restart();
}

void HeavyMachineGun::update() {}












RocketLauncher::RocketLauncher()
    : ProjectileWeapon(WEAPON_ROCKET_LAUNCHER, 5, 0.5f, 10, PROJ_EXPLOSIVE)
    , reloadTime(2.0f)
{}

RocketLauncher::~RocketLauncher() {}

void RocketLauncher::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;
    if (this->reloadTimer.getElapsedTime().asSeconds() < this->reloadTime) return;

    this->ammo--;
    
    pm->spawnExplosive(origin, dir, angle, this->damage, 3, false);
    this->reloadTimer.restart();
}

void RocketLauncher::update() {}




























FlameShot::FlameShot()
    : Weapon(WEAPON_FLAME_SHOT, 2, 10.f, 50)
    , streamLength(5)
{}

FlameShot::~FlameShot() {}

void FlameShot::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    
    
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    
    
    this->ammo--;

    
    pm->spawnFlame(origin, dir, angle, this->damage, false);

    this->fireTimer.restart();
}

void FlameShot::update() {
    
    
}



























LaserGun::LaserGun()
    : Weapon(WEAPON_LASER_GUN, 999, 0.5f, 20)
{}

LaserGun::~LaserGun() {}

void LaserGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    
    
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    this->ammo--;

    
    
    
    (void)angle;
    pm->spawnLaser(origin, dir, this->damage, false);

    this->fireTimer.restart();
}

void LaserGun::update() {
    
    
}