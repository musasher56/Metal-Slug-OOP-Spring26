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

// ------------------------------------------------------------------
// update() — Template Method (non-virtual, enforces pipeline order)
// ------------------------------------------------------------------
// Pipeline:
//   1. move(scroll)            — subclass advances position
//   2. checkTileCollision(lvl) — base tests solid tiles
//   3. checkBounds(scroll)     — base deactivates off-screen projectiles
//
// WHY check status at the top AND after each step?
//   move() can be called on a projectile that was already inactive
//   if the caller doesn't guard (common bug in student code).
//   The early return here is the single authoritative guard.
//   After checkTileCollision, we must also check status because
//   a hit might have deactivated us; calling checkBounds on a
//   deactivated projectile is harmless but wasteful.
// ------------------------------------------------------------------
void Projectile::update(float scroll, Level* lvl) {
    if (!this->status) return;

    // Step 1: Subclass advances position
    this->move(scroll);

    // Step 2: Tile collision — base handles it for ALL subclasses
    if (lvl != nullptr) {
        this->checkTileCollision(lvl);
    }

    if (!this->status) return;  // tile hit deactivated us

    // Step 3: Deactivate if it flew off screen
    this->checkBounds(scroll);
}

// ------------------------------------------------------------------
// checkTileCollision — leading-edge probe
// ------------------------------------------------------------------
// WHY test the leading edge instead of the center?
//
//   Suppose a bullet moves 15 pixels per frame and a wall is exactly
//   1 cell (e.g. 32px) thick.  If the bullet's center starts 20px
//   in front of the wall, the center will be 5px past the wall's
//   far side next frame — the center never actually lands INSIDE the
//   wall tile, so a center-test would miss it entirely.
//
//   Leading-edge probe: we test the FRONT FACE of the projectile
//   in the direction of travel.  The front face is always the first
//   part to physically contact anything, so we test there.
//
// X-axis probe:
//   Moving right (velocityX > 0): front face is the RIGHT edge → pos.x + width
//   Moving left  (velocityX < 0): front face is the LEFT edge  → pos.x
//
// Y-axis probe:
//   Moving down  (velocityY > 0): front face is the BOTTOM edge → pos.y + height
//   Moving up    (velocityY < 0): front face is the TOP edge    → pos.y
//
// We probe X and Y independently so a diagonal bullet (angle ≠ 0°, ≠ 90°)
// resolves correctly when grazing a corner.
// ------------------------------------------------------------------
void Projectile::checkTileCollision(Level* lvl) {
    if (lvl == nullptr) return;

    const int  cell = lvl->getCellSize();
    const int  PROJ_W = 8;  // projectile bounding box width  (pixels)
    const int  PROJ_H = 8;  // projectile bounding box height (pixels)

    // --- X-axis front-face ---
    float frontX = (this->velocityX >= 0.f)
        ? this->position.x + PROJ_W   // moving right → right edge
        : this->position.x;            // moving left  → left edge

    // --- Y-axis front-face ---
    float frontY = (this->velocityY >= 0.f)
        ? this->position.y + PROJ_H   // moving down  → bottom edge
        : this->position.y;            // moving up    → top edge

    // Convert pixel coordinates to tile indices
    int colX = (int)frontX / cell;
    int rowY = (int)frontY / cell;

    // Also find the row for the X probe and the col for the Y probe,
    // using the projectile CENTER on the non-probed axis.
    // This prevents false positives on tiles diagonally adjacent.
    int rowForX = (int)(this->position.y + PROJ_H / 2.f) / cell;
    int colForY = (int)(this->position.x + PROJ_W / 2.f) / cell;

    bool hitX = lvl->isSolid(rowForX, colX);
    bool hitY = lvl->isSolid(rowY, colForY);

    if (hitX || hitY) {
        // Trigger impact behaviour (explosive will override this)
        this->onImpact(nullptr, nullptr);
        this->deactivate();
    }
}

// ------------------------------------------------------------------
// checkBounds — deactivate projectiles that have left the visible area
// ------------------------------------------------------------------
// WHY a generous margin (300px) instead of exact screen edge?
//   A projectile aimed upward at 80° might be off the visible strip
//   but still close to an enemy who is also off-screen.  Shrinking the
//   margin causes premature culling.  300px is generous and not costly.
// ------------------------------------------------------------------
void Projectile::checkBounds(float scroll) {
    const float MARGIN = 300.f;

    // Off the left side of the world
    if (this->position.x + 8.f < scroll - MARGIN) {
        this->deactivate();
        return;
    }
    // Off the right side of the visible window
    if (this->position.x > scroll + SCREEN_W + MARGIN) {
        this->deactivate();
        return;
    }
    // Too high (above the level)
    if (this->position.y + 8.f < -MARGIN) {
        this->deactivate();
        return;
    }
    // Too low (below level floor — should be caught by tile collision first)
    if (this->position.y > SCREEN_H + MARGIN) {
        this->deactivate();
    }
}

// ------------------------------------------------------------------
// draw
// ------------------------------------------------------------------
void Projectile::draw(RenderWindow& window, float scroll) {
    if (!this->status) return;
    this->animation.applyToSprite(this->sprite);
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    window.draw(this->sprite);
}

// ------------------------------------------------------------------
// Accessors
// ------------------------------------------------------------------
IntRect Projectile::getBoundingBox() const {
    // Returns WORLD-SPACE coordinates for overlap testing in checkEntityCollisions().
    // WHY width=16, height=24?
    //   Bullets travel horizontally and spawn at barrel height (player mid-body).
    //   Block tops sit at their position.y which is often 20-30px below barrel height.
    //   An 8x8 box at the bullet origin misses the block top entirely.
    //   A 16x24 box extends the hitbox downward so a bullet fired at mid-player
    //   height overlaps the top face of a block it visually passes through.
    //   Width 16 also gives a small grace window on fast-moving bullets that
    //   might skip a narrow target between frames.
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        16,  // wider than 8px — less frame-skip misses on fast bullets
        24   // taller than 8px — covers downward reach to block top face
    );
}

int  Projectile::getDamage()   const { return this->damage; }
bool Projectile::isFromEnemy() const { return this->fromEnemy; }

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // Base: no-op.  ExplosiveProjectile overrides to apply blast.
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

// WHY only two lines?
//   All the heavy lifting (tile collision, bounds, status check) is in
//   Projectile::update() which the base Template Method handles.
//   This function is ONLY responsible for advancing position — nothing else.
//   velocityX/Y were set at spawn by ProjectileManager::spawnStraight().
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

// WHY accumulate gravity into velocityY before adding to position?
//   The sequence matters:
//     velY += gravity    (update velocity for this frame)
//     pos.y += velY      (move by the NOW-UPDATED velocity)
//   This is semi-implicit Euler integration.  It's slightly more stable
//   than explicit Euler (pos.y += velY; velY += gravity) — the position
//   uses the updated velocity, so the arc overshoots slightly less.
//   Consistent with how Gravity.cpp works in your codebase.
void BallisticProjectile::move(float /*scroll*/) {
    this->velocityY += this->gravity;   // gravity accumulates each frame
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

// WHY override onImpact() here instead of in move()?
//   Explosion logic is impact-triggered, not motion-triggered.
//   The base Template Method calls onImpact() when tile collision fires,
//   AND ProjectileManager::checkEntityCollisions() calls it on entity hit.
//   Both paths go through the same override — single point of logic.
void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    // Blast damage is applied by the caller (ProjectileManager or PlayState)
    // who has access to the full enemy/character arrays.
    // We leave em/cm stubs here; the blast radius is readable via blastRadius.
    (void)em; (void)cm;
    // TODO: when EnemyManager is ready, call:
    //   em->applyBlastDamage(this->position, this->blastRadius, this->damage);
}