#pragma once
#include "Entity.h"

class EnemyManager;
class CharacterManager;
class Level;
class ProjectileManager;

// ─────────────────────────────────────────────────────────────────────────────
// Projectile — abstract base for all flying game objects
//
// WHY virtual draw() on each subclass instead of a big if-else in PM::draw()?
//   If the drawing code lived in ProjectileManager and branched on projectileClass,
//   that is exactly the anti-polymorphism pattern (P1 penalty: -100).  Pushing
//   rendering DOWN into each concrete subclass means PM::draw() just calls
//   slots[i]->draw(...) and the vtable dispatches the right colored shape —
//   no type-knowledge required by the caller.
//
// WHY protected checkTileCollision / checkBounds instead of private?
//   LaserBeam must be able to skip tile-collision inside its own update().
//   Making these protected keeps them internal (not public API) while allowing
//   controlled overriding in derived classes.
// ─────────────────────────────────────────────────────────────────────────────
class Projectile : public Entity {
    // PM needs direct field access for spawn setup (position, damage, fromEnemy)
    // without forcing every field to be public — this is the UML-specified approach.
    friend class ProjectileManager;

protected:
    float velocityX;
    float velocityY;
    int   damage;
    bool  fromEnemy;
    bool  isExplosive;
    int   blastRadius;
    int   projectileClass;

public:
    Projectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Projectile();

    // update() drives the per-frame lifecycle: move → tile collision → bounds cull.
    // Subclasses may override to suppress tile collision (e.g. LaserBeam).
    virtual void update(float scroll, Level* lvl);

    // draw() is virtual so each concrete subclass renders its own placeholder shape.
    // PM::draw() calls this uniformly — the vtable handles dispatch.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    IntRect getBoundingBox() const;
    int     getDamage()      const;
    bool    isFromEnemy()    const;

    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
    void         setVelocity(float vx, float vy);

protected:
    // move() is the per-frame translation hook — pure virtual forces each leaf
    // class to declare its trajectory model explicitly.
    virtual void move(float scroll) = 0;

    // Protected so subclasses like LaserBeam can call selectively from their
    // override of update().
    void checkTileCollision(Level* lvl);
    void checkBounds(float scrollX, float scrollY);
};

// ─────────────────────────────────────────────────────────────────────────────
// StraightProjectile — constant-velocity linear trajectory (Pistol, HMG)
//
// Draws as a bright-yellow oriented rectangle — the angle is preserved from
// construction so the rect aligns with velocity direction.
// ─────────────────────────────────────────────────────────────────────────────
class StraightProjectile : public Projectile {
private:
    float angle;    // degrees; stored to orient the rect in draw()
public:
    StraightProjectile(TextureManager* texMgr, AudioManager* audMgr, float ang);
    virtual ~StraightProjectile();

    // Yellow bullet rectangle — bright enough to read against any background.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    virtual void move(float scroll);
};

// ─────────────────────────────────────────────────────────────────────────────
// BallisticProjectile — parabolic arc under simulated gravity
//
// Used as the base for ExplosiveProjectile (rocket, grenade, bomb).
// Non-explosive ballistics are not currently spawned but the class exists
// for completeness of the hierarchy and future enemy melee-throws.
// Draws as a dark-orange oval.
// ─────────────────────────────────────────────────────────────────────────────
class BallisticProjectile : public Projectile {
protected:
    float gravity;   // px per frame² — accumulates into velocityY each tick
public:
    BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BallisticProjectile();

    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    virtual void move(float scroll);
};

// ─────────────────────────────────────────────────────────────────────────────
// ExplosiveProjectile — ballistic arc that explodes on tile or entity impact
//
// Carries blastRadius for area damage; onImpact() triggers the blast.
// projectileClass distinguishes rocket (PROJ_EXPLOSIVE) from enemy bomb
// (PROJ_BOMB) — both use the same class but draw differently because that
// intra-class visual distinction lives INSIDE the class, not as an external
// type-switch in PM.  This keeps P1 penalty at zero.
// ─────────────────────────────────────────────────────────────────────────────
class ExplosiveProjectile : public BallisticProjectile {
public:
    ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ExplosiveProjectile();

    // draw() checks own projectileClass to pick rocket vs bomb color.
    // This is NOT anti-polymorphism: the check is internal to the class,
    // not an external observer switching on a foreign object's type.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    virtual void onImpact(EnemyManager* em, CharacterManager* cm);
};

// ─────────────────────────────────────────────────────────────────────────────
// FlameParticle — one short-lived particle emitted by FlameShot::fire()
//
// WHY a particle-per-fire() approach instead of a single wide hitbox?
//   Repeated fire() calls at fireRate=10/s naturally create a continuous stream
//   of 20-frame particles.  At any given tick ~3 overlap, matching the visual
//   "5-block flame stream" from the spec.  Tile collision correctly blocks the
//   flame at walls — important for the Mummy-kill mechanic.
//
// WHY extend StraightProjectile instead of Projectile directly?
//   FlameShot is NOT a ProjectileWeapon (UML constraint), but the particle it
//   EMITS does travel linearly.  Reusing StraightProjectile::move() avoids
//   duplicating the linear-movement math.  FlameParticle just adds a lifetime
//   countdown on top.
//
// The fade effect in draw() (orange → red → transparent) gives a visible cue
// that the flame is dying out, matching real flame behaviour.
// ─────────────────────────────────────────────────────────────────────────────
class FlameParticle : public StraightProjectile {
private:
    int lifetime;       // frames remaining before the particle burns out
    int maxLifetime;    // original value — used to compute the fade ratio in draw()
public:
    FlameParticle(TextureManager* texMgr, AudioManager* audMgr,
                  float ang, int frames = 20);
    virtual ~FlameParticle();

    // Fading orange-to-red rectangle — alpha drops with remaining lifetime.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    // move() advances position via parent then checks lifetime.
    // When lifetime reaches zero the particle self-deactivates — PM's
    // postEntityUpdate() then removes it from the slot array on the next frame.
    virtual void move(float scroll);
};

// ─────────────────────────────────────────────────────────────────────────────
// LaserBeam — stationary wide-hitbox projectile representing an instant raycast
//
// WHY stationary instead of very-high-velocity?
//   A bullet moving 2000 px/frame would teleport PAST enemies in the same tick
//   its position is checked — classic discrete-step tunnelling.  By keeping the
//   beam at position=0 and making getBoundingBox() span the full screen width,
//   the AABB check in PM::checkPlayerBulletHits() catches every enemy in the
//   beam path in the same frame the beam is spawned.
//
// WHY override update() (suppress tile collision)?
//   The spec says LaserGun performs a raycast — it passes through geometry.
//   The base Projectile::update() calls checkTileCollision() which would
//   deactivate the beam the moment it overlaps a solid block.  Since the beam
//   is at the barrel tip (open air), in practice this rarely fires, but the
//   override makes the intent architecturally explicit and safe.
//
// The beam flickers in draw() as lifetime counts down, giving a visible
// charge-and-fade effect even without a real sprite.
// ─────────────────────────────────────────────────────────────────────────────
class LaserBeam : public StraightProjectile {
private:
    int lifetime;    // how many frames the beam stays active and visible
    int beamDir;     // DIR_LEFT or DIR_RIGHT — determines hitbox and draw direction
public:
    LaserBeam(TextureManager* texMgr, AudioManager* audMgr,
              int dir, int frames = 5);
    virtual ~LaserBeam();

    // Override: the beam's AABB spans the full screen width in beamDir.
    // This is what makes the "instant raycast" actually hit targets on spawn.
    virtual IntRect getBoundingBox() const;

    // Override: skip tile collision so the beam passes through walls.
    virtual void update(float scroll, Level* lvl);

    // Cyan double-layer rectangle (outer glow + bright core) with alpha fade.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    // No spatial movement — move() is purely a frame-countdown timer.
    virtual void move(float scroll);
};

// ─────────────────────────────────────────────────────────────────────────────
// MeleeSlash — short-range instant-hit attack (knife / combat knife)
//
// WHY a projectile for a melee attack?
//   Reusing the existing projectile collision pipeline (PM::checkPlayerBulletHits)
//   means melee damage "just works" against all DamagableEntity targets without
//   needing a separate collision system.  The slash has a short lifetime (8 frames)
//   and a wide but short bounding box, simulating a close-range swing.
//
// The slash is stationary (like LaserBeam) but with a tiny hitbox — roughly
// 60px wide and 50px tall, positioned in front of the player.
// ─────────────────────────────────────────────────────────────────────────────
class MeleeSlash : public Projectile {
private:
    int lifetime;      // frames remaining before the slash fades
    int maxLifetime;   // original value — used to compute the fade ratio in draw()
    int slashDir;      // DIR_LEFT or DIR_RIGHT — positions the hitbox
    float slashWidth;  // hitbox width in pixels
    float slashHeight; // hitbox height in pixels
public:
    MeleeSlash(TextureManager* texMgr, AudioManager* audMgr,
               int dir, int frames = 8);
    virtual ~MeleeSlash();

    // Override: the slash's AABB is short-range in the facing direction.
    virtual IntRect getBoundingBox() const;

    // Override: skip tile collision — melee hits entities, not walls.
    virtual void update(float scroll, Level* lvl);

    // White-yellow slash visual with alpha fade.
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

protected:
    // No spatial movement — move() is purely a frame-countdown timer.
    virtual void move(float scroll);
};