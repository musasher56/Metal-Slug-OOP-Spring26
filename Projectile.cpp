#include "Projectile.h"
#include "Level.h"
#include <cmath>





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
        8,
        6
    );
}

int  Projectile::getDamage()   const { return this->damage; }
bool Projectile::isFromEnemy() const { return this->fromEnemy; }

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    (void)em; (void)cm;
}





StraightProjectile::StraightProjectile(TextureManager* texMgr,
    AudioManager* audMgr,
    float ang)
    : Projectile(texMgr, audMgr)
    , angle(ang)
{
    this->projectileClass = PROJ_STRAIGHT;

    // ── Sprite setup ─────────────────────────────────────────────────────────
    // Without this, the sprite has no texture and draw() renders nothing.
    // ProjectileManager already called loadTexture("bullet", ...) in its
    // constructor, so getTexture() is guaranteed to return a valid reference.
    Texture& t = texMgr->getTexture("bullet");
    this->sprite.setTexture(t);
    this->sprite.setScale(2.f, 2.f);
    // Rotate the sprite to match the bullet's travel direction.
    // angle=0 means horizontal right; positive angle tilts upward.
    this->sprite.setRotation(-ang);
    this->status = true;   // Mark active so draw() and update() run
}

StraightProjectile::~StraightProjectile() {}

void StraightProjectile::move(float) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}





BallisticProjectile::BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : Projectile(texMgr, audMgr)
    , gravity(0.5f)
{
    this->projectileClass = PROJ_BALLISTIC;
}

BallisticProjectile::~BallisticProjectile() {}

void BallisticProjectile::move(float) {
    this->velocityY += this->gravity;
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}





ExplosiveProjectile::ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : BallisticProjectile(texMgr, audMgr)
{
    this->isExplosive  = true;
    this->blastRadius  = 3;
    this->projectileClass = PROJ_EXPLOSIVE;
    this->status = true;

    // ── Sprite setup ─────────────────────────────────────────────────────────
    // loadTexture returns true if the file exists and loads successfully.
    // If grenade.png isn't present yet, we fall back to the bullet texture
    // with a yellow-orange tint so grenades are still visible during development.
    // YOU NEED TO ADD: resources/Sprites/grenade.png
    bool grenadeLoaded = texMgr->loadTexture("grenade", "resources/Sprites/grenade.png");

    if (grenadeLoaded) {
        Texture& gt = texMgr->getTexture("grenade");
        this->sprite.setTexture(gt);
        this->sprite.setScale(0.05f, 0.05f);
    } else {
        // Fallback: bullet texture with orange tint so it's at least visible
        Texture& bt = texMgr->getTexture("bullet");
        this->sprite.setTexture(bt);
        this->sprite.setScale(3.f, 3.f);
        this->sprite.setColor(sf::Color(255, 160, 30));  // orange tint
    }
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // Basic onImpact: deactivate the grenade.
    // Blast radius damage against nearby enemies/player goes here once
    // EnemyManager and CharacterManager expose a takeAreaDamage() method.
    // For now the grenade at least visually disappears on impact (not silently
    // ignored), and the deactivation is handled by the caller via deactivate().
    (void)em; (void)cm;
    // deactivate() is called by checkTileCollision after onImpact returns,
    // so we don't need to call it here — just leave the status for the caller.
}