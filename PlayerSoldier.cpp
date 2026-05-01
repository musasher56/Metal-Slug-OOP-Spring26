#include "PlayerSoldier.h"
#include <iostream>
#include <fstream>

// ============================================================
// PlayerSoldier
// ============================================================

PlayerSoldier::PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Soldier(texMgr, audMgr)
    , currentWeapon(nullptr)
    , pistol(nullptr)
    , currentGrenade(nullptr)
    , grenadeCount(10)      // PDF: starting grenades = 10
    , inVehicle(false)
    , currentVehicle(nullptr)
    , inventorySize(0)
    , isFat(false)
    , fatGravRadius(0.f)
    , aimController()
    , pm(nullptr)           // set later via setProjectileManager()
{
    for (int i = 0; i < 3; ++i) this->inventory[i] = nullptr;

    // WHY create Pistol here?
    //   Pistol is always available (infinite ammo, default weapon per PDF).
    //   Without this, currentWeapon is nullptr and shoot() silently does nothing.
    this->pistol        = new Pistol();
    this->currentWeapon = this->pistol;  // start with pistol equipped
}

PlayerSoldier::~PlayerSoldier() {
    // WHY null-check before delete?  Defensive programming — if constructor
    // threw partway through, some pointers may still be nullptr.
    if (this->pistol != nullptr) {
        delete this->pistol;
        this->pistol = nullptr;
    }
    // Don't double-delete if currentWeapon == pistol
    if (this->currentWeapon != nullptr && this->currentWeapon != this->pistol) {
        delete this->currentWeapon;
        this->currentWeapon = nullptr;
    }
    if (this->currentGrenade != nullptr) {
        delete this->currentGrenade;
        this->currentGrenade = nullptr;
    }
    for (int i = 0; i < this->inventorySize; ++i) {
        if (this->inventory[i] != nullptr &&
            this->inventory[i] != this->pistol) {
            delete this->inventory[i];
            this->inventory[i] = nullptr;
        }
    }
    // pm is NOT owned here — never delete it
}

// ============================================================
// setProjectileManager
// ============================================================
// Call once from PlayState after creating the player:
//
//   Marco* marco = new Marco(texMgr, audMgr);
//   marco->setProjectileManager(&pm);   // pm is your ProjectileManager
//
void PlayerSoldier::setProjectileManager(ProjectileManager* manager) {
    this->pm = manager;
}

// ============================================================
// updateAim
// ============================================================
// Call every frame from PlayState or CharacterManager:
//
//   sf::Vector2f mouseWorld(
//       Mouse::getPosition(window).x + scroll,   // convert screen→world
//       Mouse::getPosition(window).y
//   );
//   player->updateAim(mouseWorld);
//
// WHY add scroll to mouse X?
//   Mouse::getPosition returns screen-space coordinates (0..SCREEN_W).
//   Your entities live in world-space (position.x includes scroll offset).
//   Adding scroll converts mouse to world-space so the angle is correct
//   even when the camera has moved right.
// ============================================================
void PlayerSoldier::updateAim(sf::Vector2f mousePos) {
    this->aimController.update(mousePos, this->position, this->direction);
}

// ============================================================
// shoot
// ============================================================
// HOW SHOOTING WORKS — full pipeline:
//
//   1. Guard: need a weapon, ammo, and a ProjectileManager
//   2. Calculate barrel tip (spawn origin just past sprite edge)
//   3. Pass origin + direction + angle to weapon->fire()
//   4. Weapon calls pm->spawnStraight() or pm->spawnExplosive()
//   5. ProjectileManager adds it to the pool
//   6. Next frame: ProjectileManager::update() moves it
//                  ProjectileManager::draw()   renders it
//
// KEYBINDING: call shoot() from handleInput() when Space/Z is pressed.
// ============================================================
void PlayerSoldier::shoot() {
    // Guard 1: no pm set yet (PlayState hasn't called setProjectileManager)
    if (this->pm == nullptr) return;

    // Guard 2: no weapon or out of ammo
    if (this->currentWeapon == nullptr) return;
    if (!this->currentWeapon->hasAmmo())  return;

    // Step 1: Get current aim angle (0° = horizontal, 90° = straight up)
    float angle = this->aimController.getAngle();

    // Step 2: Calculate barrel tip — where the bullet spawns
    // WHY not just use this->position?
    //   position is the TOP-LEFT of the sprite.  Spawning there puts bullets
    //   at the player's feet.  calcBarrelTip offsets to the gun barrel.
    //
    // 32.f  = sprite width  in pixels (unscaled — matches your IntRect width)
    // 20.f  = barrel height from top of sprite (tune this visually)
    // If your sprite is scaled (e.g. 3.5x), the pixel offset is already in
    // world-space because position.x/y track world coords, not sprite coords.
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->direction,
        32.f,    // sprite width — adjust if your character sprite is different
        20.f     // barrel Y offset from top of sprite
    );

    // Step 3: Fire — weapon decides projectile type (straight vs explosive)
    this->currentWeapon->fire(origin, this->direction, angle, this->pm);
}

// ============================================================
// switchWeapon
// ============================================================
void PlayerSoldier::switchWeapon(Weapon* w) {
    if (w == nullptr) return;
    // Store old non-pistol weapon in inventory
    if (this->currentWeapon != nullptr &&
        this->currentWeapon != this->pistol &&
        this->inventorySize < 3)
    {
        this->inventory[this->inventorySize++] = this->currentWeapon;
    }
    this->currentWeapon = w;
}

// ============================================================
// Vehicle
// ============================================================
void PlayerSoldier::enterVehicle(Vehicle* v) {
    if (v == nullptr || this->inVehicle) return;
    this->currentVehicle = v;
    this->inVehicle = true;
    v->setDriver(this);
}

void PlayerSoldier::exitVehicle() {
    if (!this->inVehicle || this->currentVehicle == nullptr) return;
    this->currentVehicle->ejectDriver();
    this->currentVehicle = nullptr;
    this->inVehicle = false;
}

// ============================================================
// Save / Load
// ============================================================
void PlayerSoldier::saveData(std::ofstream& out) {
    if (!out.is_open()) return;
    out.write(reinterpret_cast<const char*>(&this->lives),       sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->currentHP),   sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->grenadeCount), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->inVehicle),   sizeof(bool));
    out.write(reinterpret_cast<const char*>(&this->position.x),  sizeof(float));
    out.write(reinterpret_cast<const char*>(&this->position.y),  sizeof(float));
}

void PlayerSoldier::loadData(std::ifstream& in) {
    if (!in.is_open()) return;
    in.read(reinterpret_cast<char*>(&this->lives),       sizeof(int));
    in.read(reinterpret_cast<char*>(&this->currentHP),   sizeof(int));
    in.read(reinterpret_cast<char*>(&this->grenadeCount), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->inVehicle),   sizeof(bool));
    in.read(reinterpret_cast<char*>(&this->position.x),  sizeof(float));
    in.read(reinterpret_cast<char*>(&this->position.y),  sizeof(float));
}

void PlayerSoldier::applyFannumTax(ProjectileManager* manager) {
    if (manager == nullptr || !this->isFat) return;
}

void PlayerSoldier::onDeath() {
    this->lives--;
    if (this->lives > 0) this->respawn();
}

void PlayerSoldier::updateBoundingBox() {
    this->boundingBox = IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        static_cast<int>(32 * std::abs(this->sprite.getScale().x)),
        static_cast<int>(48 * std::abs(this->sprite.getScale().y))
    );
}

// ============================================================
// Marco
// ============================================================
Marco::Marco(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , fireRateMultiplier(1.25f)
    , dualFireActive(false)
{
    Texture& tex = texMgr->getTexture("resources/Sprites/marco.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(12);
    this->animation.setLoop(true);
    this->sprite.setTexture(tex);
    this->sprite.setScale(3.5f, 3.5f);
    this->sprite.setTextureRect(IntRect(0, 0, 32, 32));
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Marco::~Marco() {}

void Marco::updateSprite() {
    int frameIndex = 0;
    this->sprite.setTextureRect(IntRect(frameIndex * 32, 0, 32, 32));
}

void Marco::activatePowerUp() {
    this->dualFireActive = true;
    this->dualFireTimer.restart();
}

// ============================================================
// Marco::handleInput
// ============================================================
// THIS IS WHERE SHOOTING IS TRIGGERED.
//
// Key bindings (Metal Slug standard):
//   Left  / Right arrow  → move
//   Space or Up arrow    → jump
//   Z  (or LCtrl)        → shoot current weapon
//   X  (or LAlt)         → throw grenade
//
// WHY check Keyboard::isKeyPressed here instead of in events?
//   isKeyPressed gives you HELD state (true every frame the key is down).
//   This is correct for weapons like HMG that fire continuously while held.
//   The fire timer inside each Weapon handles the actual rate limiting.
// ============================================================
void Marco::handleInput() {
    // Movement
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        this->setDirectionAndVelocity(DIR_LEFT);
    } else if (Keyboard::isKeyPressed(Keyboard::Right)) {
        this->setDirectionAndVelocity(DIR_RIGHT);
    } else {
        this->decelerate();
    }

    // Jump
    if (Keyboard::isKeyPressed(Keyboard::Space) ||
        Keyboard::isKeyPressed(Keyboard::Up))
    {
        this->handleJump();
    }

    // ── SHOOT ────────────────────────────────────────────────────────────
    // Z key fires the current weapon.
    // shoot() → calcBarrelTip() → currentWeapon->fire() → pm->spawnStraight()
    if (Keyboard::isKeyPressed(Keyboard::Z)) {
        this->shoot();
    }

    // Marco power-up: dual-direction fire
    // When active, shoot() fires normally AND we also fire the opposite direction
    if (this->dualFireActive && Keyboard::isKeyPressed(Keyboard::Z)) {
        if (this->pm != nullptr && this->currentWeapon != nullptr) {
            int oppositeDir = (this->direction == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
            sf::Vector2f origin = ProjectileManager::calcBarrelTip(
                this->position, oppositeDir, 32.f, 20.f
            );
            this->currentWeapon->fire(origin, oppositeDir,
                                       this->aimController.getAngle(), this->pm);
        }
        // Check if 10 seconds elapsed
        if (this->dualFireTimer.getElapsedTime().asSeconds() >= 10.f) {
            this->dualFireActive = false;
        }
    }

    // Throw grenade
    if (Keyboard::isKeyPressed(Keyboard::X)) {
        this->throwGrenade();
    }
}

void Marco::meleeAttack() {
    Soldier::meleeAttack();  // Marco's melee pierces shields (handled in collision)
}

// ============================================================
// Tarma
// ============================================================
Tarma::Tarma(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , vehicleFireRateBonus(0.25f)
    , vehicleDurabilityBonus(0.20f)
    , immunityActive(false)
{
    Texture& tex = texMgr->getTexture("resources/Sprites/Character.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(12);
    this->animation.setLoop(true);
    this->sprite.setTexture(tex);
    this->sprite.setScale(0.2f, 0.2f);
    this->sprite.setTextureRect(IntRect(0, 0, 32, 32));
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Tarma::~Tarma() {}
void Tarma::updateSprite()     { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Tarma::activatePowerUp()  { this->immunityActive = true; this->immunityTimer.restart(); }
bool Tarma::hasVehicleSurvival() const { return true; }
void Tarma::onVehicleDestroyed()       { this->exitVehicle(); }

void Tarma::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Left))       this->setDirectionAndVelocity(DIR_LEFT);
    else if (Keyboard::isKeyPressed(Keyboard::Right)) this->setDirectionAndVelocity(DIR_RIGHT);
    else                                               this->decelerate();
    if (Keyboard::isKeyPressed(Keyboard::Space) || Keyboard::isKeyPressed(Keyboard::Up))
        this->handleJump();
    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
}

// ============================================================
// Eri
// ============================================================
Eri::Eri(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , blastRadiusMultiplier(1.50f)
    , doubleGrenadeActive(false)
{
    Texture& tex = texMgr->getTexture("resources/Sprites/Character.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(12);
    this->animation.setLoop(true);
    this->sprite.setTexture(tex);
    this->sprite.setScale(0.2f, 0.2f);
    this->sprite.setTextureRect(IntRect(0, 0, 32, 32));
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Eri::~Eri() {}
void Eri::updateSprite()    { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Eri::activatePowerUp() { this->doubleGrenadeActive = true; this->doubleGrenadeTimer.restart(); }

void Eri::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Left))       this->setDirectionAndVelocity(DIR_LEFT);
    else if (Keyboard::isKeyPressed(Keyboard::Right)) this->setDirectionAndVelocity(DIR_RIGHT);
    else                                               this->decelerate();
    if (Keyboard::isKeyPressed(Keyboard::Space) || Keyboard::isKeyPressed(Keyboard::Up))
        this->handleJump();
    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
}


// ============================================================
// Fio
// ============================================================
Fio::Fio(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , ammoBonusMultiplier(1.50f)
    , fireRateMultiplier(1.10f)
    , superchargedActive(false)
{
    Texture& tex = texMgr->getTexture("resources/Sprites/Character.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(12);
    this->animation.setLoop(true);
    this->sprite.setTexture(tex);
    this->sprite.setScale(0.2f, 0.2f);
    this->sprite.setTextureRect(IntRect(0, 0, 32, 32));
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Fio::~Fio() {}
void Fio::updateSprite()    { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Fio::activatePowerUp() { this->superchargedActive = true; this->superchargedTimer.restart(); }
void Fio::pickUpWeapon()    { /* +50% ammo bonus handled on weapon pickup */ }

void Fio::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Left))       this->setDirectionAndVelocity(DIR_LEFT);
    else if (Keyboard::isKeyPressed(Keyboard::Right)) this->setDirectionAndVelocity(DIR_RIGHT);
    else                                               this->decelerate();
    if (Keyboard::isKeyPressed(Keyboard::Space) || Keyboard::isKeyPressed(Keyboard::Up))
        this->handleJump();
    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
}

// ============================================================
// REPLACE your existing throwGrenade() in PlayerSoldier.cpp with this.
// Also replace Eri::throwGrenade() with the version below.
// Everything else in PlayerSoldier.cpp stays the same.
// ============================================================

// ── Base PlayerSoldier::throwGrenade() ────────────────────────────────
// WHY 45° and not the aim angle?
//   Aim angle controls the gun barrel. Grenades are lobbed — thrown in a
//   fixed arc. 45° gives maximum range, which feels natural for a lob throw.
//   The horizontal component is determined by facing direction.
//   Damage: 20 HP per PDF.  Blast radius: 3 blocks per PDF.
void PlayerSoldier::throwGrenade() {
    if (this->pm == nullptr)          return;
    if (this->grenadeCount <= 0)      return;

    this->grenadeCount--;

    // Spawn origin: player's hands, slightly forward and above center
    // WHY 24.f Y offset? Grenades come from the hand, not the gun barrel.
    // 24px from top of 64px sprite puts it at roughly chest/hand height.
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->direction,
        32.f,    // sprite width
        24.f     // hand height from top of sprite
    );

    // WHY 45°? Maximum range for a ballistic throw.
    // The grenade arc looks natural and clears most obstacles.
    const float LOB_ANGLE  = 45.f;
    const int   NADE_DAMAGE = 20;   // PDF: grenade damage = 20 HP
    const int   BLAST_RADIUS = 3;   // PDF: blast radius = 3 blocks

    this->pm->spawnExplosive(origin, this->direction,
                              LOB_ANGLE, NADE_DAMAGE,
                              BLAST_RADIUS, false);
}

// ── Eri::throwGrenade() ───────────────────────────────────────────────
// Eri carries double grenades and her power-up throws 2 for cost of 1.
// Second grenade lands 2 blocks farther (slightly lower angle = more range).
void Eri::throwGrenade() {
    if (this->pm == nullptr)     return;
    if (this->grenadeCount <= 0) return;

    // Normal throw
    this->grenadeCount--;

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position, this->direction, 32.f, 24.f
    );

    this->pm->spawnExplosive(origin, this->direction,
                              45.f, 20, 3, false);

    // Power-up: second grenade at shallower angle = lands ~2 blocks farther
    // Cost: 1 extra grenade (costs 2 total), only if active AND enough count
    if (this->doubleGrenadeActive && this->grenadeCount >= 1) {
        this->grenadeCount--;

        // 30° = shallower arc = travels farther horizontally before landing
        // WHY 30°? tan(30°) ≈ 0.577 vs tan(45°) = 1.0 — roughly 2 blocks extra
        // range at standard throw velocity.
        this->pm->spawnExplosive(origin, this->direction,
                                  30.f, 20, 3, false);

        if (this->doubleGrenadeTimer.getElapsedTime().asSeconds() >= 10.f)
            this->doubleGrenadeActive = false;
    }
}