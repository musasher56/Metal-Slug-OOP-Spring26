#include "Grenade.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Grenade (abstract base)
//
// Grenades are NOT Projectiles — they are a separate hierarchy rooted here.
// The key difference: Projectile objects live in ProjectileManager's pool and
// are updated/drawn through it.  Grenades are owned directly by PlayerSoldier
// (or CharacterManager) and updated/drawn by whoever holds them.
//
// This separation matters for memory ownership (P3 penalty):
//   PlayerSoldier composition-owns its currentGrenade (deletes it).
//   PM does NOT touch grenades.
// ─────────────────────────────────────────────────────────────────────────────

Grenade::Grenade(int radius, int dmg)
    : position(0.f, 0.f)
    , blastRadius(radius)
    , damage(dmg)
    , velocity(0.f, 0.f)
    , active(false)
{}

Grenade::~Grenade() {}

void Grenade::update() {
    if (!this->active) return;

    // Simulate ballistic arc: accumulate gravity into Y velocity each frame.
    // Same gravity constant as BallisticProjectile (0.5f) for visual consistency.
    this->velocity.y += 0.5f;
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;

    // Ground collision: deactivate when the grenade passes below the viewport.
    // Proper tile-collision check should be added once Level is accessible here.
    if (this->position.y > 2000.f) {
        this->active = false;
    }
}

void Grenade::draw(RenderWindow& window, float scrollX, float scrollY) {
    // Base draw() renders an olive-green sphere — subclasses override colour.
    if (!this->active) return;

    sf::CircleShape body(6.f);
    body.setFillColor(sf::Color(80, 100, 30));    // olive green
    body.setOutlineColor(sf::Color(50, 70, 20));
    body.setOutlineThickness(1.5f);
    body.setOrigin(6.f, 6.f);
    body.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(body);

    // Animated rotation cue: draw a white dot offset from centre to show spin.
    // Angle is derived from velocity direction so the dot sweeps correctly.
    float rotAngle = std::atan2f(this->velocity.y, this->velocity.x);
    sf::CircleShape pin(2.f);
    pin.setFillColor(sf::Color(220, 220, 220));
    pin.setOrigin(2.f, 2.f);
    pin.setPosition(
        (this->position.x - scrollX) + std::cosf(rotAngle) * 4.f,
        (this->position.y - scrollY) + std::sinf(rotAngle) * 4.f
    );
    window.draw(pin);
}

bool         Grenade::isActive()      const { return this->active;       }
sf::Vector2f Grenade::getPosition()   const { return this->position;     }
int          Grenade::getBlastRadius()const { return this->blastRadius;  }
int          Grenade::getDamage()     const { return this->damage;       }

// ─────────────────────────────────────────────────────────────────────────────
// HandGrenade
//
// Spec: BALLISTIC arc, 5 HP in 3-block blast radius.
// Colour placeholder: olive green sphere (from base Grenade::draw).
//
// launch() computes an initial velocity that gives a natural lob arc toward
// the target.  The horizontal component is gated by a divisor so very-close
// targets still get an arc (not a flat shot).
// ─────────────────────────────────────────────────────────────────────────────

HandGrenade::HandGrenade()
    : Grenade(3, 5)
{}

HandGrenade::~HandGrenade() {}

void HandGrenade::launch(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active   = true;

    sf::Vector2f dir = target - origin;
    // Horizontal: spread over 20 frames (roughly 0.33s) for a medium lob.
    // Vertical: fixed upward kick — gravity (0.5f/frame²) brings it back down.
    this->velocity.x = dir.x / 20.f;
    this->velocity.y = -10.f;
}

void HandGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    // Blast-radius damage goes here once EnemyManager exposes
    // takeAreaDamage(origin, radius, dmg).  The blast visual is handled by
    // ProjectileManager::spawnBlast() — call it through the PM reference
    // once this class has access to PM (or pass it as a parameter to onImpact).
    (void)em;
}

// ─────────────────────────────────────────────────────────────────────────────
// FireBombGrenade
//
// Spec: BALLISTIC arc + persistent fire pool on impact.
//   Pool: radius 3 blocks, lasts 10 seconds, deals 2 HP/s.
//
// Colour placeholder: orange-red sphere (hotter than hand grenade) with
//   a persistent fire-pool rectangle rendered while poolActive.
//
// The fire pool is visual-only for now — actual per-second area damage
// requires a PhysicsZone or CollectibleManager integration.
// ─────────────────────────────────────────────────────────────────────────────

FireBombGrenade::FireBombGrenade()
    : Grenade(3, 2)
    , poolActive(false)
    , poolRadius(3)
    , poolLifetime(10.f)
{}

FireBombGrenade::~FireBombGrenade() {}

void FireBombGrenade::launch(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active   = true;

    sf::Vector2f dir = target - origin;
    this->velocity.x = dir.x / 20.f;
    this->velocity.y = -10.f;
}

void FireBombGrenade::draw(RenderWindow& window, float scrollX, float scrollY) {
    // Draw the grenade itself while in flight
    if (this->active) {
        // Orange-red sphere (hotter than HandGrenade's olive)
        sf::CircleShape body(7.f);
        body.setFillColor(sf::Color(220, 80, 10));
        body.setOutlineColor(sf::Color(180, 40, 0));
        body.setOutlineThickness(1.5f);
        body.setOrigin(7.f, 7.f);
        body.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(body);
    }

    // Draw the persistent fire pool after impact
    if (this->poolActive && this->poolDuration.getElapsedTime().asSeconds() < this->poolLifetime) {
        // Fire pool: semi-transparent red-orange rectangle scaled to poolRadius blocks.
        // cellSize placeholder = 32 (replace with Level::getCellSize() when accessible).
        const int   cellSize  = 32;
        const float poolPx    = static_cast<float>(this->poolRadius * cellSize * 2);

        sf::RectangleShape pool(sf::Vector2f(poolPx, 16.f));
        pool.setFillColor(sf::Color(255, 60, 0, 120));
        pool.setOutlineColor(sf::Color(255, 120, 0, 180));
        pool.setOutlineThickness(2.f);
        pool.setOrigin(poolPx * 0.5f, 8.f);
        pool.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(pool);
    } else if (this->poolActive) {
        // Pool expired
        this->poolActive = false;
    }
}

void FireBombGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    this->spawnFirePool();
    (void)em;
}

void FireBombGrenade::spawnFirePool() {
    // Activate the visual pool and start the duration timer.
    this->poolActive = true;
    this->poolDuration.restart();
    // Actual per-second area damage (2 HP/s for 10s) integrates with
    // the enemy manager — implement in PlayState::update() once accessible.
}