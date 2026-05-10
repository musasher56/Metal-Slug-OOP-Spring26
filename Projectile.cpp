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
{
}

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

    const int cell = lvl->getCellSize();
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
    // Generous off-screen margin keeps in-flight bullets valid slightly
    // beyond the viewport edge so fast-moving projectiles don't pop away
    // while still partially visible.
    const float MARGIN = 300.f;

    if (this->position.x + 8.f < scrollX - MARGIN) { this->deactivate(); return; }
    if (this->position.x > scrollX + SCREEN_W + MARGIN) { this->deactivate(); return; }
    if (this->position.y + 8.f < scrollY - MARGIN) { this->deactivate(); return; }
    if (this->position.y > scrollY + SCREEN_H + MARGIN) { this->deactivate(); }
}

IntRect Projectile::getBoundingBox() const {
    // Standard 12x8 box — slightly larger than the previous 8x6 to improve
    // hit detection at all angles.  LaserBeam overrides this to span the
    // full screen, and MeleeSlash uses its own slashWidth/slashHeight.
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        12, 8
    );
}

int  Projectile::getDamage()   const { return this->damage; }
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
    AudioManager* audMgr,
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

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;

    // Try to use the bullet sprite if the texture was loaded
    if (this->textureManager->loadTexture("bullet_draw", "resources/Sprites/bullet.png")) {
        sf::Texture& tex = this->textureManager->getTexture("bullet_draw");
        sf::Sprite bulletSprite;
        bulletSprite.setTexture(tex);
        float texW = static_cast<float>(tex.getSize().x);
        float texH = static_cast<float>(tex.getSize().y);
        bulletSprite.setOrigin(texW * 0.5f, texH * 0.5f);
        bulletSprite.setScale(0.2f, 0.2f);
        bulletSprite.setRotation(rot);
        bulletSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(bulletSprite);
    }
    else {
        // Fallback: yellow rectangle placeholder
        sf::RectangleShape bullet(sf::Vector2f(10.f, 4.f));
        bullet.setFillColor(sf::Color(255, 240, 40));
        bullet.setOrigin(5.f, 2.f);
        bullet.setRotation(rot);
        bullet.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(bullet);
    }
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
    this->isExplosive = true;
    this->blastRadius = 3;
    this->projectileClass = PROJ_EXPLOSIVE;
    this->status = true;
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;

    if (this->projectileClass == PROJ_BOMB) {
        // Enemy-thrown bomb — try bomb.png sprite, fallback to grey circle
        if (this->textureManager->loadTexture("bomb_draw", "resources/Sprites/bomb.png")) {
            sf::Texture& tex = this->textureManager->getTexture("bomb_draw");
            sf::Sprite bombSprite;
            bombSprite.setTexture(tex);
            bombSprite.setTextureRect(sf::IntRect(0, 0, 108, 52));
            bombSprite.setOrigin(54.f, 26.f);
            bombSprite.setScale(0.8f, 0.8f);
            bombSprite.setRotation(rot);
            bombSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(bombSprite);
        }
        else {
            sf::CircleShape bomb(7.f);
            bomb.setFillColor(sf::Color(55, 55, 55));
            bomb.setOutlineColor(sf::Color(120, 120, 120));
            bomb.setOutlineThickness(1.5f);
            bomb.setOrigin(7.f, 7.f);
            bomb.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(bomb);
        }
    }
    else {
        // Rocket / grenade — try grenade.png sprite, fallback to orange rectangle
        if (this->textureManager->loadTexture("grenade_draw", "resources/Sprites/grenade.png")) {
            sf::Texture& tex = this->textureManager->getTexture("grenade_draw");
            sf::Sprite grenadeSprite;
            grenadeSprite.setTexture(tex);
            float texW = static_cast<float>(tex.getSize().x);
            float texH = static_cast<float>(tex.getSize().y);
            grenadeSprite.setOrigin(texW * 0.5f, texH * 0.5f);
            grenadeSprite.setScale(0.08f, 0.08f);
            grenadeSprite.setRotation(rot);
            grenadeSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(grenadeSprite);
        }
        else {
            sf::RectangleShape rocket(sf::Vector2f(14.f, 7.f));
            rocket.setFillColor(sf::Color(255, 90, 10));
            rocket.setOrigin(7.f, 3.5f);
            rocket.setRotation(rot);
            rocket.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(rocket);

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
    float ratio = (float)this->lifetime / (float)this->maxLifetime;

    float sx = this->position.x - scrollX;
    float sy = this->position.y - scrollY;

    // Try to use the flame particle sprite if the texture was loaded
    if (this->textureManager->loadTexture("flame_particle", "resources/Sprites/flame_particle.png")) {
        sf::Texture& tex = this->textureManager->getTexture("flame_particle");
        sf::Sprite flameSprite;
        flameSprite.setTexture(tex);
        float texW = static_cast<float>(tex.getSize().x);
        float texH = static_cast<float>(tex.getSize().y);
        flameSprite.setOrigin(texW * 0.5f, texH * 0.5f);
        // Scale and alpha based on remaining lifetime
        float s = 0.15f * ratio + 0.05f;  // shrinks as it fades
        flameSprite.setScale(s, s);
        float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;
        flameSprite.setRotation(rot);
        // Color tint: fade from bright orange to dark red
        sf::Uint8 g = static_cast<sf::Uint8>(90.f * ratio);
        sf::Uint8 alpha = static_cast<sf::Uint8>(200.f * ratio + 55.f);
        flameSprite.setColor(sf::Color(255, g + 50 > 255 ? 255 : g + 50, 10, alpha));
        flameSprite.setPosition(sx, sy);
        window.draw(flameSprite);
    }
    else {
        // Fallback: fading orange-to-red rectangle
        sf::Uint8 g = static_cast<sf::Uint8>(90.f * ratio);
        sf::Uint8 alpha = static_cast<sf::Uint8>(200.f * ratio + 55.f);

        // Outer halo — larger, more transparent
        sf::RectangleShape halo(sf::Vector2f(12.f, 8.f));
        halo.setFillColor(sf::Color(255, g, 0, static_cast<sf::Uint8>(alpha / 2)));
        halo.setOrigin(6.f, 4.f);
        halo.setPosition(sx, sy);
        window.draw(halo);

        // Inner core — smaller, more opaque, brighter
        sf::RectangleShape core(sf::Vector2f(8.f, 5.f));
        core.setFillColor(sf::Color(255, g + 50 > 255 ? 255 : g + 50, 10, alpha));
        core.setOrigin(4.f, 2.5f);
        core.setPosition(sx, sy);
        window.draw(core);
    }
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
    this->isExplosive = false;
    // Velocity stays at (0,0) — the beam occupies its extent from the spawn frame.
}

LaserBeam::~LaserBeam() {}

IntRect LaserBeam::getBoundingBox() const {
    const float BEAM_LEN   = (float)SCREEN_W;   // beam reaches one screen width
    const float MIN_THICK  = 8.f;               // minimum AABB thickness

    // velocityX/Y is a unit direction vector set by spawnLaser().
    // If it's still (0,0) fall back to purely horizontal in beamDir.
    float vx = this->velocityX;
    float vy = this->velocityY;
    if (vx * vx + vy * vy < 0.001f) {
        vx = (this->beamDir == DIR_RIGHT) ? 1.f : -1.f;
        vy = 0.f;
    }

    float endX = this->position.x + vx * BEAM_LEN;
    float endY = this->position.y + vy * BEAM_LEN;

    float left  = (this->position.x < endX) ? this->position.x : endX;
    float right = (this->position.x > endX) ? this->position.x : endX;
    float top   = (this->position.y < endY) ? this->position.y : endY;
    float bot   = (this->position.y > endY) ? this->position.y : endY;

    // Ensure minimum thickness so the AABB is never a zero-area line
    if (bot - top < MIN_THICK) { top -= MIN_THICK * 0.5f; bot += MIN_THICK * 0.5f; }
    if (right - left < MIN_THICK) { left -= MIN_THICK * 0.5f; right += MIN_THICK * 0.5f; }

    return IntRect(
        static_cast<int>(left),
        static_cast<int>(top),
        static_cast<int>(right - left),
        static_cast<int>(bot   - top)
    );
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

    float sx = this->position.x - scrollX;
    float sy = this->position.y - scrollY;

    float vx = this->velocityX;
    float vy = this->velocityY;
    if (vx * vx + vy * vy < 0.001f) {
        vx = (this->beamDir == DIR_RIGHT) ? 1.f : -1.f;
        vy = 0.f;
    }

    float beamLen  = (float)SCREEN_W * 1.2f;
    float beamAngle = atan2f(vy, vx) * 180.f / 3.14159f;

    // Alpha fades with remaining lifetime (lifetime starts at 5)
    float ratio = (float)this->lifetime / 5.f;
    sf::Uint8 glowA = static_cast<sf::Uint8>(ratio * 100.f + 20.f);
    sf::Uint8 coreA = static_cast<sf::Uint8>(ratio * 200.f + 55.f);

    // Try to use the laser beam sprite if the texture was loaded
    if (this->textureManager->loadTexture("laser_beam", "resources/Sprites/laser_beam.png")) {
        sf::Texture& tex = this->textureManager->getTexture("laser_beam");
        float texW = static_cast<float>(tex.getSize().x);
        float texH = static_cast<float>(tex.getSize().y);

        // Outer glow (wider, dimmer)
        sf::Sprite glowSprite;
        glowSprite.setTexture(tex);
        glowSprite.setOrigin(0.f, texH * 0.5f);
        float glowScaleX = beamLen / texW;
        float glowScaleY = 12.f / texH;
        glowSprite.setScale(glowScaleX, glowScaleY);
        glowSprite.setRotation(beamAngle);
        glowSprite.setPosition(sx, sy);
        glowSprite.setColor(sf::Color(80, 200, 255, glowA));
        window.draw(glowSprite);

        // Bright core (thinner, brighter)
        sf::Sprite coreSprite;
        coreSprite.setTexture(tex);
        coreSprite.setOrigin(0.f, texH * 0.5f);
        float coreScaleX = beamLen / texW;
        float coreScaleY = 4.f / texH;
        coreSprite.setScale(coreScaleX, coreScaleY);
        coreSprite.setRotation(beamAngle);
        coreSprite.setPosition(sx, sy);
        coreSprite.setColor(sf::Color(200, 240, 255, coreA));
        window.draw(coreSprite);
    }
    else {
        // Fallback: colored rectangles
        // Outer glow
        sf::RectangleShape glow(sf::Vector2f(beamLen, 12.f));
        glow.setFillColor(Color(80, 200, 255, glowA));
        glow.setOrigin(0.f, 6.f);
        glow.setPosition(sx, sy);
        glow.setRotation(beamAngle);
        window.draw(glow);

        // Bright core
        sf::RectangleShape core(sf::Vector2f(beamLen, 4.f));
        core.setFillColor(Color(200, 240, 255, coreA));
        core.setOrigin(0.f, 2.f);
        core.setPosition(sx, sy);
        core.setRotation(beamAngle);
        window.draw(core);
    }

    // Muzzle flash at spawn point (always drawn, regardless of sprite)
    float flashR = 6.f + ratio * 4.f;
    sf::CircleShape flash(flashR);
    flash.setFillColor(Color(255, 255, 200, coreA));
    flash.setOrigin(flashR, flashR);
    flash.setPosition(sx, sy);
    window.draw(flash);
}

// ─────────────────────────────────────────────────────────────────────────────
// MeleeSlash
//
// A stationary short-range hitbox representing a knife swing.
// Reuses the projectile pipeline so melee damage works against all
// DamagableEntity targets without a separate collision system.
//
// The slash appears in front of the player, lasts 8 frames, and deals
// melee damage.  It skips tile collision (melee hits entities, not walls).
// ─────────────────────────────────────────────────────────────────────────────

MeleeSlash::MeleeSlash(TextureManager* texMgr, AudioManager* audMgr,
    int dir, int frames)
    : Projectile(texMgr, audMgr)
    , lifetime(frames)
    , maxLifetime(frames)
    , slashDir(dir)
    , slashWidth(80.f)     // increased from 70 for more reliable hit detection
    , slashHeight(60.f)    // increased from 50 for more reliable hit detection
{
    this->projectileClass = PROJ_MELEE;
    this->isExplosive = false;
    this->status = true;
}

MeleeSlash::~MeleeSlash() {}

IntRect MeleeSlash::getBoundingBox() const {
    // The slash hitbox extends in front of the player in slashDir.
    // position is set to the player's position by spawnMelee().
    if (this->slashDir == DIR_RIGHT) {
        // Slash extends to the RIGHT of the player
        return IntRect(
            static_cast<int>(this->position.x),
            static_cast<int>(this->position.y),
            static_cast<int>(this->slashWidth),
            static_cast<int>(this->slashHeight)
        );
    }
    else {
        // Slash extends to the LEFT of the player
        return IntRect(
            static_cast<int>(this->position.x - this->slashWidth),
            static_cast<int>(this->position.y),
            static_cast<int>(this->slashWidth),
            static_cast<int>(this->slashHeight)
        );
    }
}

void MeleeSlash::update(float scroll, Level* /*lvl*/) {
    // Skip tile collision — melee hits entities, not walls.
    if (!this->status) return;

    this->move(scroll);
    // No bounds check needed — melee is always on-screen with the player.
}

void MeleeSlash::move(float /*scroll*/) {
    // No spatial displacement — pure lifetime countdown.
    if (--this->lifetime <= 0) {
        this->deactivate();
    }
}

void MeleeSlash::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    float ratio = (float)this->lifetime / (float)this->maxLifetime;
    sf::Uint8 alpha = static_cast<sf::Uint8>(ratio * 200.f + 55.f);

    float sx = this->position.x - scrollX;
    float sy = this->position.y - scrollY;

    // Draw slash arc — a diagonal white-yellow line
    float startX, endX;
    if (this->slashDir == DIR_RIGHT) {
        startX = sx;
        endX = sx + this->slashWidth;
    }
    else {
        startX = sx - this->slashWidth;
        endX = sx;
    }

    // Outer slash arc (wider, dimmer)
    sf::RectangleShape slashOuter(sf::Vector2f(this->slashWidth, 6.f));
    slashOuter.setFillColor(sf::Color(255, 255, 150, static_cast<sf::Uint8>(alpha * 0.5f)));
    slashOuter.setOrigin(0.f, 3.f);
    float midY = sy + this->slashHeight * 0.5f;
    if (this->slashDir == DIR_RIGHT) {
        slashOuter.setPosition(sx, midY);
    }
    else {
        slashOuter.setPosition(sx - this->slashWidth, midY);
    }
    // Slight rotation for arc feel
    float rotAngle = (1.f - ratio) * 30.f - 15.f;  // swings from -15 to +15 degrees
    if (this->slashDir == DIR_LEFT) rotAngle = -rotAngle;
    slashOuter.setRotation(rotAngle);
    window.draw(slashOuter);

    // Inner slash core (brighter, thinner)
    sf::RectangleShape slashCore(sf::Vector2f(this->slashWidth * 0.8f, 3.f));
    slashCore.setFillColor(sf::Color(255, 255, 220, alpha));
    slashCore.setOrigin(0.f, 1.5f);
    if (this->slashDir == DIR_RIGHT) {
        slashCore.setPosition(sx + this->slashWidth * 0.1f, midY);
    }
    else {
        slashCore.setPosition(sx - this->slashWidth * 0.9f, midY);
    }
    slashCore.setRotation(rotAngle);
    window.draw(slashCore);

    // Impact flash at tip
    float flashR = 4.f * ratio;
    sf::CircleShape tipFlash(flashR);
    tipFlash.setFillColor(sf::Color(255, 255, 180, alpha));
    tipFlash.setOrigin(flashR, flashR);
    if (this->slashDir == DIR_RIGHT) {
        tipFlash.setPosition(endX, midY);
    }
    else {
        tipFlash.setPosition(startX, midY);
    }
    window.draw(tipFlash);
}