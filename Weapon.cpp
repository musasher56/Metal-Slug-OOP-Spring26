#include "Weapon.h"
#include "ProjectileManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Weapon (abstract base)
// ─────────────────────────────────────────────────────────────────────────────

Weapon::Weapon(int weaponType, int dmg, float rate, int amm)
    : ammo(amm), fireRate(rate), damage(dmg), type(weaponType)
{}

Weapon::~Weapon() {}

bool  Weapon::hasAmmo()     const { return this->ammo > 0 || this->ammo == -1; }
int   Weapon::getAmmo()     const { return this->ammo;     }
int   Weapon::getDamage()   const { return this->damage;   }
float Weapon::getFireRate() const { return this->fireRate; }
int   Weapon::getType()     const { return this->type;     }

void Weapon::addAmmo(int amount) {
    if (amount < 0) return;
    // ammo == -1 is the sentinel for infinite ammo (Pistol) — leave it alone.
    if (this->ammo != -1) this->ammo += amount;
}

void Weapon::setFireRate(float r) {
    // Guard against nonsensical values.  A rate of 0 would make 1/fireRate = inf,
    // locking the weapon permanently.  Negative would reverse the time check.
    if (r > 0.f) this->fireRate = r;
}

// ─────────────────────────────────────────────────────────────────────────────
// ProjectileWeapon (abstract intermediate)
// ─────────────────────────────────────────────────────────────────────────────

ProjectileWeapon::ProjectileWeapon(int wt, int dmg, float rate, int amm, int pc)
    : Weapon(wt, dmg, rate, amm), projectileClass(pc)
{}

ProjectileWeapon::~ProjectileWeapon() {}

// ─────────────────────────────────────────────────────────────────────────────
// Pistol
//
// Infinite ammo (ammo == -1), fast fire rate (4/s), straight trajectory.
// Strategy role: lightest, always available, the "fallback" weapon.
// Colour in-game: bright yellow bullet (StraightProjectile::draw).
// ─────────────────────────────────────────────────────────────────────────────

Pistol::Pistol()
    : ProjectileWeapon(WEAPON_PISTOL, 3, 4.f, -1, PROJ_STRAIGHT)
    , infinite(true)
{}

Pistol::~Pistol() {}

void Pistol::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    // Cadence gate: 1/fireRate seconds must elapse between shots.
    // Clock::getElapsedTime() is measured in real seconds against the SFML clock.
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    pm->spawnStraight(origin, dir, angle, this->damage, false);
    this->fireTimer.restart();
}

void Pistol::update() {
    // Pistol has no state that needs per-frame maintenance (no reload,
    // no overheating, infinite ammo).  Kept as a no-op virtual to satisfy the
    // pure-virtual contract from Weapon::update() = 0.
}

// ─────────────────────────────────────────────────────────────────────────────
// HeavyMachineGun
//
// Hold-to-fire, 8 shots/sec, finite ammo (100 rounds).
// Strategy role: high DPS at close-to-medium range.
// Colour: same yellow bullet as Pistol — same PROJ_STRAIGHT class.
//   (If you want visual distinction: subclass StraightProjectile as HMGBullet
//    with a slightly larger rect and white-yellow colour, then add spawnHMGBullet()
//    to PM.  For now, single-colour keeps the pool simple.)
// ─────────────────────────────────────────────────────────────────────────────

HeavyMachineGun::HeavyMachineGun()
    : ProjectileWeapon(WEAPON_HMG, 3, 8.f, 100, PROJ_STRAIGHT)
{}

HeavyMachineGun::~HeavyMachineGun() {}

void HeavyMachineGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    // Consume one round per burst — ammo depletion is handled before spawn so
    // a weapon going empty mid-burst never fires a "free" last shot.
    this->ammo--;
    pm->spawnStraight(origin, dir, angle, this->damage, false);
    this->fireTimer.restart();
}

void HeavyMachineGun::update() {}

// ─────────────────────────────────────────────────────────────────────────────
// RocketLauncher
//
// Single-shot with a 2-second reload, fires an explosive ballistic round.
// Strategy role: area-of-effect burst damage; slow but devastating.
// Colour: fiery orange capsule with bright nose cone (ExplosiveProjectile::draw).
//
// Note: fireRate is stored but the reload is gated separately via reloadTimer.
// The spec says "2s reload" — that maps directly to reloadTime = 2.0f seconds.
// ─────────────────────────────────────────────────────────────────────────────

RocketLauncher::RocketLauncher()
    : ProjectileWeapon(WEAPON_ROCKET_LAUNCHER, 5, 0.5f, 10, PROJ_EXPLOSIVE)
    , reloadTime(2.0f)
{}

RocketLauncher::~RocketLauncher() {}

void RocketLauncher::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;
    if (this->reloadTimer.getElapsedTime().asSeconds() < this->reloadTime) return;

    this->ammo--;
    // blastRadius = 3 blocks — matches spec "3-block blast radius"
    pm->spawnExplosive(origin, dir, angle, this->damage, 3, false);
    this->reloadTimer.restart();
}

void RocketLauncher::update() {}

// ─────────────────────────────────────────────────────────────────────────────
// FlameShot
//
// WHY FlameShot does NOT extend ProjectileWeapon:
//   The spec is explicit: "NOT a ProjectileWeapon: stream-based, 5 blocks, kills
//   Mummy instantly."  The inheritance hierarchy expresses capability: a
//   ProjectileWeapon IS-A weapon that fires discrete projectile objects.
//   FlameShot's flame is a continuous damage stream, not a discrete projectile.
//   This distinction shows up architecturally — FlameShot::fire() calls
//   pm->spawnFlame() which spawns FlameParticle objects, but the conceptual
//   model is a stream, not individual shots.
//
// HOW the stream works (Strategy pattern in action):
//   PlayerSoldier calls currentWeapon->fire(origin, dir, angle, pm).
//   The vtable dispatches here.  PM is used for particle spawning and collision
//   detection without PlayerSoldier needing to know about FlameParticle.
//   The stream visual accumulates at runtime: fireRate=10/sec × 20 frame lifetime
//   = ~3 overlapping particles at steady state = visible orange stream.
//
// Kills Mummy Warriors:
//   FlameParticle::projectileClass == PROJ_FLAME.  MummyWarrior::takeDamageFrom()
//   should check this tag (implement there) to allow kill.  The tag is already
//   set in FlameParticle's constructor — no Weapon-side changes needed later.
//
// Colour: fading orange-to-red rectangle (FlameParticle::draw).
// ─────────────────────────────────────────────────────────────────────────────

FlameShot::FlameShot()
    : Weapon(WEAPON_FLAME_SHOT, 2, 10.f, 50)
    , streamLength(5)
{}

FlameShot::~FlameShot() {}

void FlameShot::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    // Fire-rate gate (same cadence mechanism as ProjectileWeapon subclasses).
    // Cadence: 10 particles/sec → one every 0.1s → steady stream when held.
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    // Consume stream fuel.  One ammo unit = one burst = one FlameParticle.
    // Balances the high rate — 50 ammo × 10/sec = 5 continuous seconds of flame.
    this->ammo--;

    // Delegate to PM — keeps all projectile creation logic in one place.
    pm->spawnFlame(origin, dir, angle, this->damage, false);

    this->fireTimer.restart();
}

void FlameShot::update() {
    // No per-frame state to maintain on the weapon side.
    // FlameParticle objects in PM handle their own lifetime countdown.
}

// ─────────────────────────────────────────────────────────────────────────────
// LaserGun
//
// WHY LaserGun does NOT extend ProjectileWeapon:
//   The spec says "raycast beam, instant kill" — a raycast is NOT a projectile.
//   It has no flight time, no trajectory, no in-world position that advances
//   per frame.  Inheriting ProjectileWeapon would be a false IS-A relationship.
//
// HOW the instant-hit is achieved (architecture deep-dive):
//   LaserBeam is a special Projectile subclass with velocity=(0,0).  Its
//   getBoundingBox() override returns an IntRect spanning SCREEN_W pixels in
//   the fire direction.  In the SAME FRAME the beam is spawned, PM's
//   checkPlayerBulletHits() iterates slots and finds the LaserBeam's giant AABB
//   overlapping every enemy in the beam's path — all take damage immediately.
//   No high-velocity tunnelling, no frame-skip miss.
//
//   The beam persists for 5 frames (visible cyan flash), then self-deactivates
//   via LaserBeam::move()'s lifetime countdown.  The second frame's collision
//   check finds the same targets if they're still alive — to prevent double-hit,
//   enemies should have brief invincibility frames after taking damage (standard
//   Metal Slug mechanic, implement in DamagableEntity::takeDamageFrom()).
//
// Damage = 999: effectively one-shots every non-boss enemy.
// Colour: double-layer cyan beam with muzzle flash (LaserBeam::draw).
// ─────────────────────────────────────────────────────────────────────────────

LaserGun::LaserGun()
    : Weapon(WEAPON_LASER_GUN, 999, 0.5f, 20)
{}

LaserGun::~LaserGun() {}

void LaserGun::fire(sf::Vector2f origin, int dir, float angle, ProjectileManager* pm) {
    if (!pm || !this->hasAmmo()) return;

    // Fire-rate gate: 1/0.5 = 2-second cooldown between beams.
    // Prevents chaining instant-kill beams — the long cooldown is intentional.
    if (this->fireTimer.getElapsedTime().asSeconds() < 1.f / this->fireRate) return;

    this->ammo--;

    // Angle is ignored for the laser — the beam fires perfectly horizontal in
    // the aim direction.  Vertical-aim support can be added when real sprites
    // are ready (spawn a beam with a rotated bounding box or sweep vertically).
    (void)angle;
    pm->spawnLaser(origin, dir, this->damage, false);

    this->fireTimer.restart();
}

void LaserGun::update() {
    // LaserBeam objects in PM manage their own lifetime.
    // No weapon-side per-frame state needed.
}