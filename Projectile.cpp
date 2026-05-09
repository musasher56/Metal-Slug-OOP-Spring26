#include "Projectile.h"
#include "Level.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Projectile (abstract base)
// ─────────────────────────────────────────────────────────────────────────────

Projectile::Projectile(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , damage(1)
    , fromEnemy(false)
    , isExplosive(false)
    , blastRadius(0)
    , projectileClass(0)
{}

Projectile::~Projectile() {}

void Projectile::setVelocity(float vx, float vy) {
    this->velocityX = vx;
    this->velocityY = vy;
}

// Standard per-frame lifecycle: advance position, check tile collision, cull OOB.
// Subclasses (LaserBeam) may override this to suppress tile collision.
void Projectile::update(float scroll, Level* lvl) {
    if (!this->status) return;

    this->move(scroll);

    if (lvl != nullptr) {
        this->checkTileCollision(lvl);
    }

    if (!this->status) return;

    this->checkBounds(scroll, 0.f);
}

// Default draw() — subclasses replace this entirely.
// Kept in the base so any accidental un-overridden concrete class at least
// renders something visible during debugging.
void Projectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    // Magenta — highly visible "placeholder for a placeholder" indicator.
    // If you see a magenta rect in-game, a projectile subclass is missing draw().
    sf::RectangleShape dbg(sf::Vector2f(8.f, 6.f));
    dbg.setFillColor(sf::Color(255, 0, 255));
    dbg.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(dbg);
}

void Projectile::checkTileCollision(Level* lvl) {
    if (lvl == nullptr) return;

    const int cell   = lvl->getCellSize();
    const int PROJ_W = 8;
    const int PROJ_H = 8;

    // Sample the leading edge in each axis independently.
    // Two separate checks let us detect corner-grazes correctly.
    float frontX = (this->velocityX >= 0.f)
        ? this->position.x + PROJ_W
        : this->position.x;

    float frontY = (this->velocityY >= 0.f)
        ? this->position.y + PROJ_H
        : this->position.y;

    int colX  = (int)frontX / cell;
    int rowY  = (int)frontY / cell;

    int rowForX = (int)(this->position.y + PROJ_H / 2.f) / cell;
    int colForY = (int)(this->position.x + PROJ_W / 2.f) / cell;

    bool hitX = lvl->isSolid(rowForX, colX);
    bool hitY = lvl->isSolid(rowY,    colForY);

    if (hitX || hitY) {
        this->onImpact(nullptr, nullptr);
        this->deactivate();
    }
}

void Projectile::checkBounds(float scrollX, float scrollY) {
    // Generous off-screen margin keeps in-flight bullets valid slightly
    // beyond the viewport edge so fast-moving projectiles don't pop away
    // while still partially visible.
    const float MARGIN = 300.f;

    if (this->position.x + 8.f < scrollX - MARGIN)             { this->deactivate(); return; }
    if (this->position.x        > scrollX + SCREEN_W + MARGIN) { this->deactivate(); return; }
    if (this->position.y + 8.f < scrollY - MARGIN)             { this->deactivate(); return; }
    if (this->position.y        > scrollY + SCREEN_H + MARGIN) { this->deactivate(); }
}

IntRect Projectile::getBoundingBox() const {
    // Standard 8x6 box — LaserBeam overrides this to span the full screen.
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        8, 6
    );
}

int  Projectile::getDamage()   const { return this->damage;    }
bool Projectile::isFromEnemy() const { return this->fromEnemy; }

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // Base: no-op.  ExplosiveProjectile overrides to deal blast damage.
    (void)em; (void)cm;
}

// ─────────────────────────────────────────────────────────────────────────────
// StraightProjectile
// Represents: Pistol bullets, HMG rounds
// Placeholder colour: bright yellow  (distinguishes from orange explosives and
//                                     cyan laser — immediately readable)
// ─────────────────────────────────────────────────────────────────────────────

StraightProjectile::StraightProjectile(TextureManager* texMgr,
                                       AudioManager*   audMgr,
                                       float           ang)
    : Projectile(texMgr, audMgr)
    , angle(ang)
{
    this->projectileClass = PROJ_STRAIGHT;
    this->status = true;
}

StraightProjectile::~StraightProjectile() {}

void StraightProjectile::move(float /*scroll*/) {
    // Simple Euler integration — velocity is set once on spawn and never changes.
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

void StraightProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    // 10×4 yellow rectangle oriented to velocity direction.
    // Using RectangleShape (no texture needed) keeps this working even before
    // any sprite sheets are integrated.
    sf::RectangleShape bullet(sf::Vector2f(10.f, 4.f));
    bullet.setFillColor(sf::Color(255, 240, 40));   // bright yellow
    bullet.setOrigin(5.f, 2.f);                     // rotate around centre

    // atan2 → degrees: aligns the long axis with the velocity vector.
    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;
    bullet.setRotation(rot);
    bullet.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(bullet);
}

// ─────────────────────────────────────────────────────────────────────────────
// BallisticProjectile
// Gravity accumulates into velocityY each tick, producing a parabolic arc.
// No direct weapon fires a plain ballistic (all explosives go through
// ExplosiveProjectile), but the class exists as a reusable physics base.
// Placeholder colour: dark orange
// ─────────────────────────────────────────────────────────────────────────────

BallisticProjectile::BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : Projectile(texMgr, audMgr)
    , gravity(0.5f)
{
    this->projectileClass = PROJ_BALLISTIC;
}

BallisticProjectile::~BallisticProjectile() {}

void BallisticProjectile::move(float /*scroll*/) {
    // Gravity accumulates into Y each frame — this simulates constant downward
    // acceleration (roughly 0.5 * g for the game's exaggerated ballistic feel).
    this->velocityY += this->gravity;
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

void BallisticProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    sf::RectangleShape shot(sf::Vector2f(9.f, 6.f));
    shot.setFillColor(sf::Color(200, 100, 20));  // dark orange
    shot.setOrigin(4.5f, 3.f);

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;
    shot.setRotation(rot);
    shot.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(shot);
}

// ─────────────────────────────────────────────────────────────────────────────
// ExplosiveProjectile
// Extends BallisticProjectile with a blast radius and onImpact() detonation.
//
// WHY internal projectileClass check in draw() doesn't violate P1:
//   P1 prohibits external type-dispatch ("if enemy->getType == REBEL, do X").
//   Here the class is checking its OWN internal tag to decide ITS OWN visual.
//   This is equivalent to a member function consulting its own private state —
//   semantically no different from checking a bool flag.  The vtable still
//   dispatched correctly to get HERE; there's no external switch.
//
// Colours:
//   PROJ_EXPLOSIVE (rocket): red-orange elongated oval (rocket body shape)
//   PROJ_BOMB      (enemy):  dark grey circle (metal bomb silhouette)
// ─────────────────────────────────────────────────────────────────────────────

ExplosiveProjectile::ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : BallisticProjectile(texMgr, audMgr)
{
    this->isExplosive     = true;
    this->blastRadius     = 3;
    this->projectileClass = PROJ_EXPLOSIVE;
    this->status          = true;
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;

    if (this->projectileClass == PROJ_BOMB) {
        // Enemy-thrown bomb — dark grey, round silhouette
        sf::CircleShape bomb(7.f);
        bomb.setFillColor(sf::Color(55, 55, 55));
        bomb.setOutlineColor(sf::Color(120, 120, 120));
        bomb.setOutlineThickness(1.5f);
        bomb.setOrigin(7.f, 7.f);
        bomb.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(bomb);
    } else {
        // Rocket / grenade — fiery red-orange elongated capsule rotated to arc
        sf::RectangleShape rocket(sf::Vector2f(14.f, 7.f));
        rocket.setFillColor(sf::Color(255, 90, 10));
        rocket.setOrigin(7.f, 3.5f);
        rocket.setRotation(rot);
        rocket.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(rocket);

        // Small bright-white nose cone tip
        sf::CircleShape nose(2.5f);
        nose.setFillColor(sf::Color(255, 200, 100));
        nose.setOrigin(2.5f, 2.5f);
        float noseOffX = std::cosf(rot * 3.14159f / 180.f) * 7.f;
        float noseOffY = std::sinf(rot * 3.14159f / 180.f) * 7.f;
        nose.setPosition((this->position.x - scrollX) + noseOffX,
                         (this->position.y - scrollY) + noseOffY);
        window.draw(nose);
    }
}

void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // Blast-radius damage against nearby entities is handled by
    // ProjectileManager::checkPlayerBulletHits() which calls spawnBlast() for
    // explosion visuals.  Actual area-of-effect health deduction will hook in
    // once EnemyManager exposes takeAreaDamage(origin, radius, dmg).
    (void)em; (void)cm;
}

// ─────────────────────────────────────────────────────────────────────────────
// FlameParticle
//
// One individual particle emitted by FlameShot::fire().
// At fireRate 10/sec each particle lasts 20 frames → about 3 overlap at once
// → continuous stream visual even though each fire() call only spawns 1.
//
// Extends StraightProjectile for its linear move() — FlameParticle just wraps
// it with a lifetime countdown so particles burn out after travelling ~3 blocks.
//
// The alpha-fade in draw() communicates "the flame is weakening" intuitively
// and helps players gauge the effective range of the weapon.
// ─────────────────────────────────────────────────────────────────────────────

FlameParticle::FlameParticle(TextureManager* texMgr, AudioManager* audMgr,
                             float ang, int frames)
    : StraightProjectile(texMgr, audMgr, ang)
    , lifetime(frames)
    , maxLifetime(frames)
{
    // Override the class tag so PM and collision code can identify flame hits.
    // Needed for the Mummy kill-condition (only fire or explosives deal lethal
    // damage to Mummy Warriors — the MummyWarrior::takeDamage() reads this tag).
    this->projectileClass = PROJ_FLAME;
}

FlameParticle::~FlameParticle() {}

void FlameParticle::move(float scroll) {
    // Advance spatially via parent's linear integrator, THEN count down.
    StraightProjectile::move(scroll);

    // Self-deactivate when the particle burns out.
    // PM's postEntityUpdate() removes deactivated slots on the next frame.
    if (--this->lifetime <= 0) {
        this->deactivate();
    }
}

void FlameParticle::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    // Lifetime ratio [0,1] — 1.0 at birth, 0.0 when expired.
    // Used to fade both green channel (orange→red) and alpha (fully opaque→gone).
    float ratio = (float)this->lifetime / (float)this->maxLifetime;

    sf::Uint8 g     = static_cast<sf::Uint8>(90.f  * ratio);  // green component fades out
    sf::Uint8 alpha = static_cast<sf::Uint8>(200.f * ratio + 55.f);

    // Outer halo — larger, more transparent
    sf::RectangleShape halo(sf::Vector2f(12.f, 8.f));
    halo.setFillColor(sf::Color(255, g, 0, static_cast<sf::Uint8>(alpha / 2)));
    halo.setOrigin(6.f, 4.f);
    halo.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(halo);

    // Inner core — smaller, more opaque, brighter
    sf::RectangleShape core(sf::Vector2f(8.f, 5.f));
    core.setFillColor(sf::Color(255, g + 50 > 255 ? 255 : g + 50, 10, alpha));
    core.setOrigin(4.f, 2.5f);
    core.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(core);
}

// ─────────────────────────────────────────────────────────────────────────────
// LaserBeam
//
// A stationary projectile whose getBoundingBox() spans the full screen width.
// "Stationary" is the key architectural trick: the AABB check in PM hits every
// enemy in the beam's path in the same frame it's spawned (no tunnelling).
// The beam deactivates after `lifetime` frames, producing a visible flash.
//
// update() override: skips tile collision (spec says raycast, passes through
// solid geometry).  Bounds check still applies to deactivate beams that were
// spawned off-screen in edge cases.
// ─────────────────────────────────────────────────────────────────────────────

LaserBeam::LaserBeam(TextureManager* texMgr, AudioManager* audMgr,
                     int dir, int frames)
    : StraightProjectile(texMgr, audMgr, 0.f)
    , lifetime(frames)
    , beamDir(dir)
{
    this->projectileClass = PROJ_BEAM;
    this->isExplosive     = false;
    // Velocity stays at (0,0) — the beam occupies its extent from the spawn frame.
}

LaserBeam::~LaserBeam() {}

IntRect LaserBeam::getBoundingBox() const {
    // The beam extends SCREEN_W pixels from the barrel tip in beamDir.
    // Height of 8 gives a forgiving hit window without feeling unfair.
    // Vertically centred on the barrel: subtract 4 from Y for symmetry.
    if (this->beamDir == DIR_RIGHT) {
        return IntRect(
            static_cast<int>(this->position.x),
            static_cast<int>(this->position.y) - 4,
            SCREEN_W, 8
        );
    } else {
        return IntRect(
            static_cast<int>(this->position.x) - SCREEN_W,
            static_cast<int>(this->position.y) - 4,
            SCREEN_W, 8
        );
    }
}

void LaserBeam::update(float scroll, Level* /*lvl*/) {
    // lvl is intentionally discarded — laser raycasts through solid tiles.
    // We still need the move() call for the lifetime countdown,
    // and bounds check to cull a beam that was somehow spawned off-screen.
    if (!this->status) return;

    this->move(scroll);
    if (this->status) {
        // Use a very large margin so an on-screen beam is never culled early.
        this->checkBounds(scroll, 0.f);
    }
}

void LaserBeam::move(float /*scroll*/) {
    // No spatial displacement — pure lifetime countdown.
    if (--this->lifetime <= 0) {
        this->deactivate();
    }
}

void LaserBeam::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    // Flicker intensity drops as the beam expires.
    // lifetime starts at 5, so ratio goes 1.0 → 0.2 → 0 (then deactivated).
    float ratio = (float)this->lifetime / 5.f;
    if (ratio > 1.f) ratio = 1.f;

    sf::Uint8 alpha     = static_cast<sf::Uint8>(180.f * ratio + 75.f);
    sf::Uint8 coreAlpha = static_cast<sf::Uint8>(230.f * ratio + 25.f);

    float beamLen = static_cast<float>(SCREEN_W);

    // Start X in screen space
    float startX = (this->beamDir == DIR_RIGHT)
        ? (this->position.x - scrollX)
        : (this->position.x - scrollX - beamLen);
    float y      = this->position.y - scrollY;

    // ── Outer glow: wide, semi-transparent cyan ───────────────────────────────
    sf::RectangleShape glow(sf::Vector2f(beamLen, 8.f));
    glow.setFillColor(sf::Color(0, 220, 255, alpha));
    glow.setOrigin(0.f, 4.f);
    glow.setPosition(startX, y);
    window.draw(glow);

    // ── Inner core: narrow, nearly opaque, near-white ─────────────────────────
    sf::RectangleShape core(sf::Vector2f(beamLen, 3.f));
    core.setFillColor(sf::Color(180, 255, 255, coreAlpha));
    core.setOrigin(0.f, 1.5f);
    core.setPosition(startX, y);
    window.draw(core);

    // ── Muzzle flash: small circle at barrel end ───────────────────────────────
    float flashX = (this->beamDir == DIR_RIGHT)
        ? this->position.x - scrollX
        : this->position.x - scrollX;

    sf::CircleShape flash(6.f);
    flash.setFillColor(sf::Color(200, 255, 255, coreAlpha));
    flash.setOrigin(6.f, 6.f);
    flash.setPosition(flashX, y);
    window.draw(flash);
}