#include "ProjectileManager.h"
#include "Level.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

ProjectileManager::ProjectileManager(TextureManager* t, AudioManager* a)
    : activeCount(0)
    , texMgr(t)
    , audMgr(a)
    , blastCount(0)
{
    for (int i = 0; i < MAX_PROJ; i++) this->slots[i] = nullptr;

    // Preload projectile textures so each draw() call finds them cached.
    // The "bullet" key is used by StraightProjectile's base constructor;
    // "bullet_draw", "grenade_draw", "bomb_draw" are used by the polymorphic
    // draw() methods in StraightProjectile and ExplosiveProjectile.
    this->texMgr->loadTexture("bullet", "resources/Sprites/bullet.png");
    this->texMgr->loadTexture("bullet_draw", "resources/Sprites/bullet.png");
    this->texMgr->loadTexture("grenade_draw", "resources/Sprites/grenade.png");
    this->texMgr->loadTexture("bomb_draw", "resources/Sprites/bomb.png");

    // ── Weapon projectile sprite placeholders ──────────────────────────────
    // These are the filenames you should use for your custom sprites.
    // Create the PNG files with these EXACT names in resources/Sprites/:
    //
    //   flame_particle.png   — Flamethrower / FlameShot projectile sprite
    //   laser_beam.png       — LaserGun beam sprite
    //
    // If the file doesn't exist, a fallback colored rectangle is drawn instead.
    // Recommended sizes:
    //   flame_particle.png:  16x16 or 32x32 pixels (will be scaled at runtime)
    //   laser_beam.png:      64x8 or 128x8 pixels (elongated beam shape)
    if (!this->texMgr->loadTexture("flame_particle", "resources/Sprites/flame_particle.png")) {
        this->texMgr->makeColorTexture("flame_particle", sf::Color(255, 120, 0));  // orange placeholder
    }
    if (!this->texMgr->loadTexture("laser_beam", "resources/Sprites/laser_beam.png")) {
        this->texMgr->makeColorTexture("laser_beam", sf::Color(80, 200, 255));  // cyan placeholder
    }

    // Blast animation pool — initialise every slot so postEntityUpdate() can
    // safely call anim.update() without a null check.
    // If blast.png doesn't exist yet, makeColorTexture creates a solid orange
    // 4×4 placeholder registered under "blast" — getTexture("blast") then finds
    // it on the first call and never spams the WARN log.
    if (!this->texMgr->loadTexture("blast", "resources/Sprites/blast.png")) {
        this->texMgr->makeColorTexture("blast", sf::Color(255, 140, 0));  // orange placeholder
    }
    for (int i = 0; i < MAX_BLASTS; i++) {
        this->blasts[i].active = false;
        this->blasts[i].anim.setTexture(&this->texMgr->getTexture("blast"));
        this->blasts[i].anim.setFrameCount(3);
        this->blasts[i].anim.setFrameDelay(6);
        this->blasts[i].anim.setFrameRect(0, 75, 339, 308, 278);
        this->blasts[i].anim.setFrameRect(1, 463, 353, 457, 287);
        this->blasts[i].anim.setFrameRect(2, 984, 304, 505, 400);
        this->blasts[i].anim.setLoop(false);
    }
}

ProjectileManager::~ProjectileManager() {
    this->clearAll();
}

void ProjectileManager::clearAll() {
    for (int i = 0; i < this->activeCount; i++) {
        delete this->slots[i];
        this->slots[i] = nullptr;
    }
    this->activeCount = 0;

    for (int i = 0; i < MAX_BLASTS; i++) {
        this->blasts[i].active = false;
    }
    this->blastCount = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility helpers
// ─────────────────────────────────────────────────────────────────────────────
sf::Vector2f ProjectileManager::calcBarrelTip(sf::Vector2f entityPos,
                                               int          dir,
                                               float        spriteWidth,
                                               float        barrelOffsetY)
{
    const float GAP = 4.f;
    // Facing RIGHT: barrel exits from the RIGHT edge of the sprite body.
    // entityPos.x is the LEFT edge, so add spriteWidth to reach the right side.
    // Facing LEFT: barrel exits from the LEFT edge = entityPos.x − GAP.
    float bx = (dir == DIR_RIGHT)
        ? entityPos.x + spriteWidth + GAP
        : entityPos.x - GAP;
    return sf::Vector2f(bx, entityPos.y + barrelOffsetY);
}
void ProjectileManager::angleToVelocity(float angle, int dir, float speed,
    float& outVX, float& outVY)
{
    // angle is in degrees where 0 = horizontal, positive = upward.
    // Convert to standard math radians: positive Y is downward in SFML, so negate.
    float rad = angle * 3.14159f / 180.f;
    outVX = (dir == DIR_RIGHT ? 1.f : -1.f) * std::cosf(rad) * speed;
    outVY = -std::sinf(rad) * speed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Spawn helpers
// Each creates the correct Projectile subclass, writes its fields through the
// friend relationship, and adds it to the dense slot array.
// ─────────────────────────────────────────────────────────────────────────────

void ProjectileManager::spawnStraight(sf::Vector2f origin, int dir,
    float angle, int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    StraightProjectile* p = new StraightProjectile(this->texMgr, this->audMgr, angle);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 18.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

void ProjectileManager::spawnExplosive(sf::Vector2f origin, int dir,
    float angle, int dmg,
    int blastRadius, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;
    p->blastRadius = blastRadius;
    // projectileClass stays PROJ_EXPLOSIVE (set in constructor)

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 8.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

void ProjectileManager::spawnBomb(sf::Vector2f origin, int dir,
    float angle, int dmg, int blastRadius,
    bool fromEnemy, float speed)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;
    p->blastRadius = blastRadius;
    p->projectileClass = PROJ_BOMB;   // tag differentiates grey bomb from orange rocket

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, speed, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

// ── spawnFlame ─────────────────────────────────────────────────────────────
// One FlameParticle per call.  At FlameShot fireRate = 10/sec, particles
// accumulate ~3 deep (each lives 20 frames = 0.33 sec) → stream visual.
//
// WHY one particle and not a burst?
//   A burst of 3-5 per fire() call would saturate the 64-slot pool in
//   6 fire calls (0.6 sec) and block further fire.  One per call gives a
//   steady stream without hogging pool slots.
//   If you WANT a denser visual, lower maxLifetime to 12 or raise fireRate.
void ProjectileManager::spawnFlame(sf::Vector2f origin, int dir,
    float angle, int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    // 30-frame lifetime: at 60fps and velocity 10px/f, the particle travels
    // 300px ≈ 6.25 blocks before burning out — covers good range for a stream weapon.
    FlameParticle* fp = new FlameParticle(this->texMgr, this->audMgr, angle, 30);
    fp->position = origin;
    fp->fromEnemy = fromEnemy;
    fp->damage = dmg;

    // Moderate speed — fast enough to reach enemies, slow enough to see the stream.
    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 10.f, vx, vy);
    fp->setVelocity(vx, vy);

    this->slots[this->activeCount++] = fp;
}

// ── spawnLaser ─────────────────────────────────────────────────────────────
// One stationary LaserBeam.  Its overridden getBoundingBox() spans SCREEN_W
// pixels in the fire direction — the AABB check in checkPlayerBulletHits()
// will hit every enemy in that horizontal strip in the SAME FRAME the beam
// is spawned.  Damage = 999 to match the spec's "instant kill" requirement.
//
// The beam lives 5 frames, flickering in draw() — visible flash without
// requiring a separate animation system.
void ProjectileManager::spawnLaser(sf::Vector2f origin, int dir, float angle,
                                    int dmg, bool fromEnemy) {
    if (this->activeCount >= MAX_PROJ) return;

    LaserBeam* p = new LaserBeam(this->texMgr, this->audMgr, dir, 5);
    p->position        = origin;
    p->damage          = dmg;
    p->fromEnemy       = fromEnemy;
    p->isExplosive     = false;
    p->blastRadius     = 0;
    p->projectileClass = PROJ_BEAM;
    p->status          = true;

    // Convert aim angle to a unit direction vector.
    // This is NOT a movement velocity — LaserBeam never moves.
    // It is a DIRECTION ONLY, read by getBoundingBox() and draw()
    // to orient the beam toward the mouse position.
    float ux, uy;
    this->angleToVelocity(angle, dir, 1.f, ux, uy);  // speed=1 → unit vector
    p->setVelocity(ux, uy);

    this->slots[this->activeCount++] = p;
}


// ── spawnMelee ────────────────────────────────────────────────────────────
// One short-range MeleeSlash.  Position is set to the player's front so the
// slash hitbox overlaps enemies within knife range.  The slash lasts 8 frames
// and deals the specified damage on hit.
void ProjectileManager::spawnMelee(sf::Vector2f origin, int dir,
                                    int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    MeleeSlash* p = new MeleeSlash(this->texMgr, this->audMgr, dir, 8);
    p->position        = origin;
    p->damage          = dmg;
    p->fromEnemy       = fromEnemy;
    p->isExplosive     = false;
    p->blastRadius     = 0;
    p->projectileClass = PROJ_MELEE;
    p->status          = true;

    // Set a directional velocity so that checkPlayerBulletHits() computes
    // the correct bulletDir for shield-blocking logic.  MeleeSlash::move()
    // ignores velocity (it's a lifetime-countdown projectile), so this value
    // is ONLY used for the bulletDir calculation:
    //   bulletDir = (velocityX >= 0) ? 1 : -1
    // Without this, velocityX was always 0, making bulletDir always 1,
    // which caused ShieldedSoldier to block ALL melee attacks from the front
    // regardless of which side the player was actually on.
    p->velocityX = (dir == DIR_RIGHT) ? 1.f : -1.f;
    p->velocityY = 0.f;

    this->slots[this->activeCount++] = p;
}


void ProjectileManager::spawnBlast(float x, float y) {
    for (int i = 0; i < MAX_BLASTS; i++) {
        if (!this->blasts[i].active) {
            this->blasts[i].x = x;
            this->blasts[i].y = y;
            this->blasts[i].active = true;
            this->blasts[i].anim.reset();
            return;
        }
    }
    // If all blast slots are full, silently drop — gameplay continues correctly,
    // we just lose one explosion visual.  Not worth crashing or expanding.
}

// ─────────────────────────────────────────────────────────────────────────────
// Per-frame pipeline
// ─────────────────────────────────────────────────────────────────────────────

void ProjectileManager::update(float scroll, Level* lvl) {
    (void)lvl;
    // Call virtual move() on each active projectile.
    // The vtable dispatches:  StraightProjectile  → linear advance
    //                         BallisticProjectile → gravity + advance
    //                         FlameParticle       → advance + lifetime countdown
    //                         LaserBeam           → lifetime countdown only
    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p != nullptr && p->status) {
            p->move(scroll);
        }
    }
}

void ProjectileManager::postEntityUpdate(float scrollX, float scrollY, Level* lvl) {
    int i = 0;
    while (i < this->activeCount) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->status) {
            this->removeAt(i);
            continue;
        }

        if (lvl != nullptr) {
            // Record impact position before checkTileCollision deactivates the slot.
            float  impactX = p->position.x;
            float  impactY = p->position.y;
            bool   wasExplosive = p->isExplosive;

            // Friend access: calls Projectile::checkTileCollision() directly.
            // LaserBeam and MeleeSlash skip tile collision — they override update()
            // to suppress it.  But postEntityUpdate() calls it externally.
            // For LaserBeam (velocity=0, positioned at barrel in open air) this
            // check always returns false in practice.  For MeleeSlash, we skip
            // the check entirely since melee hits entities, not walls.
            if (p->projectileClass != PROJ_MELEE) {
                p->checkTileCollision(lvl);
            }

            if (!p->status && wasExplosive) {
                this->spawnBlast(impactX, impactY);
            }
        }

        if (!p->status) { this->removeAt(i); continue; }

        // Friend access: bounds-cull off-screen projectiles.
        // Skip for MeleeSlash — it stays with the player and dies by lifetime.
        if (p->projectileClass != PROJ_MELEE) {
            p->checkBounds(scrollX, scrollY);
        }
        if (!p->status) { this->removeAt(i); continue; }

        i++;
    }

    // Tick blast animations; deactivate finished ones.
    for (int b = 0; b < MAX_BLASTS; b++) {
        if (this->blasts[b].active) {
            this->blasts[b].anim.update();
            if (this->blasts[b].anim.isFinished()) {
                this->blasts[b].active = false;
            }
        }
    }
}

// ── draw ───────────────────────────────────────────────────────────────────
// PURE POLYMORPHIC DISPATCH.  No type-checking whatsoever — the vtable does
// all the routing.  Adding a new projectile subclass with its own visual only
// requires overriding draw() in that class; zero changes needed here.
//
//   StraightProjectile::draw()  → bright yellow rectangle (Pistol / HMG)
//   BallisticProjectile::draw() → dark orange oval
//   ExplosiveProjectile::draw() → orange capsule (rocket) or grey sphere (bomb)
//   FlameParticle::draw()       → fading orange-to-red double rectangle
//   LaserBeam::draw()           → cyan double-layer beam + muzzle flash
void ProjectileManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p != nullptr && p->getStatus()) {
            // Virtual dispatch — no switch, no if-else on type.
            p->draw(window, scrollX, scrollY);
        }
    }

    // ── Blast effect animations (explosion sprite frames) ───────────────────
    // These are separate from Projectile lifetime — they outlive the projectile
    // and are owned/rendered directly by PM.
    for (int b = 0; b < MAX_BLASTS; b++) {
        if (!this->blasts[b].active) continue;

        BlastEffect& blast = this->blasts[b];
        Sprite blastSprite;
        blast.anim.applyToSprite(blastSprite);

        IntRect rect = blastSprite.getTextureRect();
        blastSprite.setOrigin(
            static_cast<float>(rect.width) * 0.5f,
            static_cast<float>(rect.height) * 0.5f);
        blastSprite.setScale(0.8f, 0.8f);
        blastSprite.setPosition(blast.x - scrollX, blast.y - 85.f - scrollY);
        window.draw(blastSprite);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Collision queries
// ─────────────────────────────────────────────────────────────────────────────

int ProjectileManager::checkEntityCollisions(DamagableEntity** targets, int targetCount)
{
    int totalDamage = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();
        bool    hit = false;

        for (int e = 0; e < targetCount && !hit; e++) {
            if (targets[e] == nullptr || !targets[e]->isAlive()) continue;

            IntRect entBox = targets[e]->getBoundingBox();

            bool overlapX = (projBox.left < entBox.left + entBox.width) &&
                (projBox.left + projBox.width > entBox.left);
            bool overlapY = (projBox.top < entBox.top + entBox.height) &&
                (projBox.top + projBox.height > entBox.top);

            if (overlapX && overlapY) {
                if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);
                targets[e]->takeDamage(proj->getDamage());
                totalDamage += proj->getDamage();
                proj->onImpact(nullptr, nullptr);
                proj->deactivate();
                hit = true;
            }
        }

        if (!proj->getStatus()) this->removeAt(p);
        else                    p++;
    }

    return totalDamage;
}

int ProjectileManager::checkPlayerBulletHits(DamagableEntity** targets, int targetCount)
{
    int hits = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }
        if (proj->isFromEnemy()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();
        bool    hit = false;

        for (int e = 0; e < targetCount && !hit; e++) {
            if (targets[e] == nullptr || !targets[e]->isAlive()) continue;

            IntRect entBox = targets[e]->getBoundingBox();

            bool overlapX = (projBox.left < entBox.left + entBox.width) &&
                (projBox.left + projBox.width > entBox.left);
            bool overlapY = (projBox.top < entBox.top + entBox.height) &&
                (projBox.top + projBox.height > entBox.top);

            if (overlapX && overlapY) {
                if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);

                // LaserBeam (PROJ_BEAM) hits ALL enemies in one pass — its wide
                // AABB means the inner loop keeps running until targetCount.
                // We don't break on LaserBeam; the inner `!hit` guard does that
                // for normal bullets (one hit = projectile consumed).
                // For laser: deactivate is called by the lifetime countdown, not here.
                int bulletDir = (proj->velocityX >= 0.f) ? 1 : -1;
                targets[e]->takeDamageFrom(proj->getDamage(), bulletDir);
                hits++;

                // Only deactivate non-beam, non-melee projectiles on hit.
                // LaserBeam must persist for its full lifetime to hit all targets.
                // MeleeSlash also persists so it can hit multiple nearby enemies.
                if (proj->projectileClass != PROJ_BEAM &&
                    proj->projectileClass != PROJ_MELEE) {
                    proj->deactivate();
                    hit = true;
                }
            }
        }

        if (!proj->getStatus()) this->removeAt(p);
        else                    p++;
    }

    return hits;
}

bool ProjectileManager::checkEnemyBulletHitPlayer(DamagableEntity* player)
{
    if (player == nullptr || !player->isAlive()) return false;

    IntRect playerBox = player->getBoundingBox();

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }
        if (!proj->isFromEnemy()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();

        bool overlapX = (projBox.left < playerBox.left + playerBox.width) &&
            (projBox.left + projBox.width > playerBox.left);
        bool overlapY = (projBox.top < playerBox.top + playerBox.height) &&
            (projBox.top + projBox.height > playerBox.top);

        if (overlapX && overlapY) {
            if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);
            player->takeDamage(proj->getDamage());
            proj->deactivate();
            if (!proj->getStatus()) this->removeAt(p);
            return true;
        }

        p++;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void ProjectileManager::removeAt(int i) {
    // O(1) compact removal: copy tail slot into the hole, set tail to nullptr.
    // This preserves array density for tight iteration in update() and draw().
    delete this->slots[i];
    this->slots[i] = nullptr;
    this->activeCount--;

    if (i < this->activeCount) {
        this->slots[i] = this->slots[this->activeCount];
        this->slots[this->activeCount] = nullptr;
    }
}