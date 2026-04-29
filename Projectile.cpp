#include "Projectile.h"
#include <cmath>

// ========== Projectile Implementation ==========

Projectile::Projectile(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , damage(1)
    , fromEnemy(false)
    , isExplosive(false)
    , blastRadius(0)
    , projectileClass(PROJ_STRAIGHT)
{}

Projectile::~Projectile() {}

void Projectile::draw(RenderWindow& window, float scroll) {
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    this->animation.applyToSprite(this->sprite);
    window.draw(this->sprite);
}

IntRect Projectile::getBoundingBox() const {
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        8,   // Default projectile width
        8    // Default projectile height
    );
}

int Projectile::getDamage() const {
    return this->damage;
}

bool Projectile::isFromEnemy() const {
    return this->fromEnemy;
}

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // WHY: Base impact handling - can be overridden by explosive projectiles
    (void)em;
    (void)cm;
}

// ========== StraightProjectile Implementation ==========

StraightProjectile::StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang)
    : Projectile(texMgr, audMgr)
    , angle(ang)
{
    this->projectileClass = PROJ_STRAIGHT;
}

StraightProjectile::~StraightProjectile() {}

void StraightProjectile::update(float scroll) {
    // WHY: Constant-velocity linear trajectory
    float speed = 15.f;
    this->velocityX = speed * cosf(this->angle * 3.14159f / 180.f);
    this->velocityY = speed * sinf(this->angle * 3.14159f / 180.f);
    
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

// ========== BallisticProjectile Implementation ==========

BallisticProjectile::BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : Projectile(texMgr, audMgr)
    , gravity(0.5f)
    , initialVelocityX(0.f)
    , initialVelocityY(0.f)
    , timeAlive(0.f)
{
    this->projectileClass = PROJ_BALLISTIC;
}

BallisticProjectile::~BallisticProjectile() {}

void BallisticProjectile::update(float scroll) {
    // WHY: Parabolic arc under simulated gravity
    this->timeAlive += 0.016f;  // Approximate frame time
    
    this->velocityX = this->initialVelocityX;
    this->velocityY = this->initialVelocityY + this->gravity * this->timeAlive * 60.f;
    
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

// ========== ExplosiveProjectile Implementation ==========

ExplosiveProjectile::ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : BallisticProjectile(texMgr, audMgr)
{
    this->isExplosive = true;
    this->blastRadius = 3;  // 3-block blast radius
    this->projectileClass = PROJ_EXPLOSIVE;
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // WHY: Apply blast damage to all enemies/characters in radius
    if (em != nullptr) {
        // em->applyBlastDamage(this->position, this->blastRadius, this->damage);
    }
    if (cm != nullptr) {
        // cm->applyBlastDamage(this->position, this->blastRadius, this->damage);
    }
}
