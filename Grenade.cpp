#include "Grenade.h"

// ========== Grenade Implementation ==========

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
    
    // Apply gravity to velocity
    this->velocity.y += 0.5f;
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
}

void Grenade::draw(RenderWindow& window, float scroll) {
    if (!this->active) return;
    
    // Draw grenade sprite at position
    // Actual implementation needs sprite
    (void)window;
    (void)scroll;
}

bool Grenade::isActive() const {
    return this->active;
}

sf::Vector2f Grenade::getPosition() const {
    return this->position;
}

int Grenade::getBlastRadius() const {
    return this->blastRadius;
}

int Grenade::getDamage() const {
    return this->damage;
}

// ========== HandGrenade Implementation ==========

HandGrenade::HandGrenade()
    : Grenade(3, 5)  // 3-block radius, 5 HP damage
{}

HandGrenade::~HandGrenade() {}

void HandGrenade::throw(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active = true;
    
    // Calculate ballistic arc velocity
    sf::Vector2f direction = target - origin;
    direction.x /= 20.f;  // Scale for throw strength
    direction.y = -10.f;   // Initial upward velocity
    
    this->velocity = direction;
}

void HandGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    
    // WHY: Apply blast damage to enemies in radius
    // Actual implementation needs EnemyManager to check nearby enemies
    if (em != nullptr) {
        // em->applyBlastDamage(this->position, this->blastRadius, this->damage);
    }
}

// ========== FireBombGrenade Implementation ==========

FireBombGrenade::FireBombGrenade()
    : Grenade(3, 2)  // 3-block radius, 2 HP/s DoT
    , poolActive(false)
    , poolRadius(3)
    , poolLifetime(10.f)
{}

FireBombGrenade::~FireBombGrenade() {}

void FireBombGrenade::throw(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active = true;
    
    // Calculate ballistic arc velocity
    sf::Vector2f direction = target - origin;
    direction.x /= 20.f;
    direction.y = -10.f;
    
    this->velocity = direction;
}

void FireBombGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    
    // WHY: Spawn persistent fire pool on impact
    this->spawnFirePool();
    
    // Also apply initial blast damage
    if (em != nullptr) {
        // em->applyBlastDamage(this->position, this->blastRadius, this->damage);
    }
}

void FireBombGrenade::spawnFirePool() {
    this->poolActive = true;
    this->poolDuration.restart();
    
    // WHY: Fire pool persists for 10 seconds, dealing 2 HP/s
    // Actual implementation needs collision detection with enemies in area
}
