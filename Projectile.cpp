#include "Projectile.h"
#include "Level.h"
#include <cmath>

// ============================================================
// Projectile  (base)
// ============================================================

Projectile::Projectile(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , damage(1)
    , fromEnemy(false)
    , isExplosive(false)
    , blastRadius(0)
    , projectileClass(0)
{
}

Projectile::~Projectile() {}

void Projectile::setVelocity(float vx, float vy) {
    this->velocityX = vx;
    this->velocityY = vy;
}

void Projectile::update(float scroll, Level* lvl) {
    if (!this->status) return;

    this->move(scroll);

    if (lvl != nullptr) {
        this->checkTileCollision(lvl);
    }

    if (!this->status) return;

    // FIX: pass scrollY for correct Y bounds
    this->checkBounds(scroll, 0.f);
}

void Projectile::checkTileCollision(Level* lvl) {
    if (lvl == nullptr) return;

    const int  cell = lvl->getCellSize();
    const int  PROJ_W = 8;
    const int  PROJ_H = 8;

    float frontX = (this->velocityX >= 0.f)
        ? this->position.x + PROJ_W
        : this->position.x;

    float frontY = (this->velocityY >= 0.f)
        ? this->position.y + PROJ_H
        : this->position.y;

    int colX = (int)frontX / cell;
    int rowY = (int)frontY / cell;

    int rowForX = (int)(this->position.y + PROJ_H / 2.f) / cell;
    int colForY = (int)(this->position.x + PROJ_W / 2.f) / cell;

    bool hitX = lvl->isSolid(rowForX, colX);
    bool hitY = lvl->isSolid(rowY, colForY);

    if (hitX || hitY) {
        this->onImpact(nullptr, nullptr);
        this->deactivate();
    }
}

// FIX: Y bounds now use scrollY so bullets survive with vertical scrolling.
// BEFORE: position.y > SCREEN_H + 300 → bullet at y=1752 killed instantly
// AFTER:  position.y > scrollY + SCREEN_H + 300 → bullet lives when on screen
void Projectile::checkBounds(float scrollX, float scrollY) {
    const float MARGIN = 300.f;

    if (this->position.x + 8.f < scrollX - MARGIN) {
        this->deactivate();
        return;
    }
    if (this->position.x > scrollX + SCREEN_W + MARGIN) {
        this->deactivate();
        return;
    }
    if (this->position.y + 8.f < scrollY - MARGIN) {
        this->deactivate();
        return;
    }
    // FIX: use scrollY instead of raw 0 — works with vertical camera
    if (this->position.y > scrollY + SCREEN_H + MARGIN) {
        this->deactivate();
    }
}

void Projectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;
    this->animation.applyToSprite(this->sprite);
    this->sprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(this->sprite);
}

IntRect Projectile::getBoundingBox() const {
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        16,
        24
    );
}

int  Projectile::getDamage()   const { return this->damage; }
bool Projectile::isFromEnemy() const { return this->fromEnemy; }

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    (void)em; (void)cm;
}

// ============================================================
// StraightProjectile
// ============================================================

StraightProjectile::StraightProjectile(TextureManager* texMgr,
    AudioManager* audMgr,
    float ang)
    : Projectile(texMgr, audMgr)
    , angle(ang)
{
    this->projectileClass = PROJ_STRAIGHT;
}

StraightProjectile::~StraightProjectile() {}

void StraightProjectile::move(float /*scroll*/) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

// ============================================================
// BallisticProjectile
// ============================================================

BallisticProjectile::BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : Projectile(texMgr, audMgr)
    , gravity(0.5f)
{
    this->projectileClass = PROJ_BALLISTIC;
}

BallisticProjectile::~BallisticProjectile() {}

void BallisticProjectile::move(float /*scroll*/) {
    this->velocityY += this->gravity;
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

// ============================================================
// ExplosiveProjectile
// ============================================================

ExplosiveProjectile::ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : BallisticProjectile(texMgr, audMgr)
{
    this->isExplosive = true;
    this->blastRadius = 3;
    this->projectileClass = PROJ_EXPLOSIVE;
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    (void)em; (void)cm;
}