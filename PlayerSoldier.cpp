#include "PlayerSoldier.h"
#include <iostream>
#include <fstream>
#include <cmath>   // std::abs for float (as opposed to <cstdlib> abs for int)




PlayerSoldier::PlayerSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Soldier(texMgr, audMgr)
    , currentWeapon(nullptr)
    , pistol(nullptr)
    , currentGrenade(nullptr)
    , grenadeCount(10)
    , inVehicle(false)
    , currentVehicle(nullptr)
    , inventorySize(0)
    , isFat(false)
    , fatGravRadius(0.f)
    , aimController()
    , pm(nullptr)
    , enemyBulletHits(0)
{
    for (int i = 0; i < 3; ++i) this->inventory[i] = nullptr;

    this->pistol = new Pistol();
    this->currentWeapon = this->pistol;
}

PlayerSoldier::~PlayerSoldier() {

    if (this->pistol != nullptr) {
        delete this->pistol;
        this->pistol = nullptr;
    }

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
}

void PlayerSoldier::setProjectileManager(ProjectileManager* manager) {
    this->pm = manager;
}

void PlayerSoldier::updateAim(sf::Vector2f mousePos) {
    this->aimController.update(mousePos, this->position, this->direction);
}

void PlayerSoldier::shoot() {

    if (this->pm == nullptr) return;

    if (this->currentWeapon == nullptr) return;
    if (!this->currentWeapon->hasAmmo())  return;

    float angle = this->aimController.getAngle();

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->direction,
        60.f,
        52.f
    );

    this->currentWeapon->fire(origin, this->direction, angle, this->pm);
}

void PlayerSoldier::switchWeapon(Weapon* w) {
    if (w == nullptr) return;

    if (this->currentWeapon != nullptr &&
        this->currentWeapon != this->pistol &&
        this->inventorySize < 3)
    {
        this->inventory[this->inventorySize++] = this->currentWeapon;
    }
    this->currentWeapon = w;
}

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

void PlayerSoldier::saveData(std::ofstream& out) {
    if (!out.is_open()) return;
    out.write(reinterpret_cast<const char*>(&this->lives), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->currentHP), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->grenadeCount), sizeof(int));
    out.write(reinterpret_cast<const char*>(&this->inVehicle), sizeof(bool));
    out.write(reinterpret_cast<const char*>(&this->position.x), sizeof(float));
    out.write(reinterpret_cast<const char*>(&this->position.y), sizeof(float));
}

void PlayerSoldier::loadData(std::ifstream& in) {
    if (!in.is_open()) return;
    in.read(reinterpret_cast<char*>(&this->lives), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->currentHP), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->grenadeCount), sizeof(int));
    in.read(reinterpret_cast<char*>(&this->inVehicle), sizeof(bool));
    in.read(reinterpret_cast<char*>(&this->position.x), sizeof(float));
    in.read(reinterpret_cast<char*>(&this->position.y), sizeof(float));
}

void PlayerSoldier::applyFannumTax(ProjectileManager* manager) {
    if (manager == nullptr || !this->isFat) return;
}

void PlayerSoldier::onDeath() {
    this->lives--;
    this->enemyBulletHits = 0;
    if (this->lives > 0) this->respawn();
}

void PlayerSoldier::takeDamage(int amount) {
    if (amount < 0) return;
    // Skip damage during invincibility frames (granted after respawn)
    if (this->isInvincible) return;

    if (this->transformState != nullptr) {

    }

    this->enemyBulletHits += amount;

    while (this->enemyBulletHits >= HITS_PER_HEART) {
        this->enemyBulletHits -= HITS_PER_HEART;
        this->currentHP -= 1;
    }

    if (this->currentHP < 0) this->currentHP = 0;

    if (this->currentHP == 0) {
        this->onDeath();
    }

    this->health = this->currentHP;
}

void PlayerSoldier::updateBoundingBox() {
    // scaleX/Y may be negative (direction flip) — abs() gives the magnitude.
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);

    // physW / physH are set per character in their constructor to their actual
    // average frame dimensions.  Multiplying by scale gives the on-screen size.
    // Because every character's  physH * scale ≈ 143px  (normalised in ctor),
    // the box height is the same for all four characters → no teleport on switch.
    int w = static_cast<int>(this->physW * scaleX);
    int h = static_cast<int>(this->physH * scaleY);

    // The box is anchored at position.x.
    // When facing left the sprite extends LEFT (negative scale), so shift the
    // box left by its width so it still covers the visible pixels.
    int left = 0;
    if (this->direction == DIR_LEFT) {
        left = -w;
    }
    this->boundingBox = IntRect(left, 0, w, h);
}

// ── PlayerSoldier::draw() ──────────────────────────────────────────────
// Replaces Soldier::draw() so we can:
//   1. Drive the walk animation (already loaded into this->animation)
//   2. Flip the sprite horizontally when facing left
//
// WHY negative scale for flip:
//   SFML builds a 2D transform matrix.  Scale is applied around the sprite's
//   origin (default top-left = (0,0)).  Negating X maps every pixel at column
//   c to column -c, which is equivalent to a horizontal mirror through the
//   origin.  No extra texture is needed — one sprite sheet, one flip.
//
// WHY std::abs before applying:
//   Calling this every frame without abs() would toggle the sign each tick,
//   making the sprite flicker between left and right every frame.  We always
//   start from the positive magnitude and then choose the sign intentionally.
// ──────────────────────────────────────────────────────────────────────
void PlayerSoldier::draw(RenderWindow& window, float scrollX, float scrollY) {

    // Advance the walk animation clock only while the character is
    // actually moving OR airborne (same rule as Soldier::draw).
    if (this->velocityX != 0.f || !this->onGround) {
        this->animation.update();
    } else {
        // Freeze on frame 0 when idle so the player stands still
        // instead of mid-stride.  Frame 0 is always valid regardless
        // of how many frames the walk cycle has.
        this->animation.currentFrame = 0;
        this->animation.clock.restart();
    }

    // Push the current frame's IntRect onto the sprite and rebind its texture
    this->animation.applyToSprite(this->sprite);

    // ── Direction flip ──
    // std::abs guarantees we start from the positive scale value, then
    // we choose the sign.  DIR_RIGHT = face right = no flip (+scaleX).
    // DIR_LEFT = face left = flip (-scaleX mirrors through Y-axis).
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    if (this->direction == DIR_RIGHT) {
        this->sprite.setScale(scaleX, scaleY);   // no flip
    } else {
        this->sprite.setScale(-scaleX, scaleY);  // horizontal mirror
    }

    // Position in screen-space (world pos minus camera scroll)
    this->sprite.setPosition(this->position.x - scrollX,
                             this->position.y - scrollY);
    window.draw(this->sprite);
}

// ... rest of the file (Marco, Tarma, Eri, Fio, throwGrenade) remains UNCHANGED


Marco::Marco(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , fireRateMultiplier(1.25f)
    , dualFireActive(false)
{
    Texture& tex = texMgr->getTexture("resources/Sprites/marco.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(12);
    this->animation.setLoop(true);














    this->animation.setDisplayCrop(0, 0, 32, 0);
    this->sprite.setTexture(tex);
    this->sprite.setScale(3.5f, 3.5f);

    this->sprite.setTextureRect(IntRect(0, 0, 36, 41));
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Marco::~Marco() {}

void Marco::updateSprite() {
    int frameIndex = 0;

    this->sprite.setTextureRect(IntRect(frameIndex * 36, 0, 36, 41));
}

void Marco::activatePowerUp() {
    this->dualFireActive = true;
    this->dualFireTimer.restart();
}

















void Marco::handleInput() {








    if (Keyboard::isKeyPressed(Keyboard::X)) {
        this->shoot();
    }



    if (this->dualFireActive && Keyboard::isKeyPressed(Keyboard::X)) {
        if (this->pm != nullptr && this->currentWeapon != nullptr) {
            int oppositeDir = (this->direction == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
            sf::Vector2f origin = ProjectileManager::calcBarrelTip(
                this->position, oppositeDir, 36.f, 20.f
            );
            this->currentWeapon->fire(origin, oppositeDir,
                this->aimController.getAngle(), this->pm);
        }

        if (this->dualFireTimer.getElapsedTime().asSeconds() >= 10.f) {
            this->dualFireActive = false;
        }
    }


    if (Keyboard::isKeyPressed(Keyboard::C)) {
        this->throwGrenade();
    }
}

void Marco::meleeAttack() {
    Soldier::meleeAttack();
}




Tarma::Tarma(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , vehicleFireRateBonus(0.25f)
    , vehicleDurabilityBonus(0.20f)
    , immunityActive(false)
{
    // ── Load Tarma's sprite sheet ────────────────────────────────────────
    // File: resources/Sprites/Tarma_Roving.png   (background removed)
    // Walk strip: rows 341-366 of the sheet, 16 frames facing RIGHT
    Texture& tex = texMgr->getTexture("resources/Sprites/Tarma_Roving.png");

    // ── Configure walkAnim with exact per-frame pixel rects ─────────────
    // setFrameRect(index, x, y, w, h):
    //   x,y = top-left pixel of this frame in the texture
    //   w,h = frame dimensions (NOT uniform — Metal Slug frames vary)
    // Setting even one rect flips useExplicitFrames = true inside Animation,
    // so applyToSprite() uses our rects instead of auto-dividing the texture.
    this->walkAnim.setTexture(&tex);
    this->walkAnim.setFrameCount(16);
    this->walkAnim.setFrameDelay(5);   // 5 * 16ms = 80ms/frame → ~12fps, smooth walk
    this->walkAnim.setLoop(true);

    // Coordinates measured pixel-exactly via automated bounding-box detection
    this->walkAnim.setFrameRect( 0,   5, 344, 21, 21);
    this->walkAnim.setFrameRect( 1,  32, 344, 21, 21);
    this->walkAnim.setFrameRect( 2,  59, 344, 20, 21);
    this->walkAnim.setFrameRect( 3,  86, 345, 20, 20);
    this->walkAnim.setFrameRect( 4, 112, 344, 21, 20);
    this->walkAnim.setFrameRect( 5, 139, 344, 21, 21);
    this->walkAnim.setFrameRect( 6, 167, 345, 21, 19);
    this->walkAnim.setFrameRect( 7, 194, 344, 21, 21);
    this->walkAnim.setFrameRect( 8, 221, 344, 21, 20);
    this->walkAnim.setFrameRect( 9, 247, 345, 23, 18);
    this->walkAnim.setFrameRect(10, 276, 345, 22, 19);
    this->walkAnim.setFrameRect(11, 304, 345, 22, 18);
    this->walkAnim.setFrameRect(12, 333, 344, 21, 20);
    this->walkAnim.setFrameRect(13, 359, 344, 21, 21);
    this->walkAnim.setFrameRect(14, 385, 344, 21, 20);
    this->walkAnim.setFrameRect(15, 413, 344, 21, 21);

    // ── Point the base animation at walkAnim's data ──────────────────────
    // Soldier::draw() drives this->animation directly. We copy walkAnim's
    // config into it so the existing draw pipeline works without changes.
    // (Alternative: override draw() here too — but reusing animation is simpler.)
    this->animation = this->walkAnim;

    // ── Sprite setup ─────────────────────────────────────────────────────
    // Scale chosen so on-screen height matches Marco:
    //   Marco baseline = 41px * 3.5  = 143.5px
    //   Tarma target   = 21px * 6.8f = 142.8px  ≈ same
    // physW/physH tell updateBoundingBox() the logical frame size so the
    // hitbox height is 21*6.8 ≈ 143px — identical to Marco's 41*3.5 = 143px.
    // No height difference → no Y-teleport when Z-switching characters.
    this->physW = 21;
    this->physH = 21;

    this->sprite.setTexture(tex);
    this->sprite.setScale(6.8f, 6.8f);

    // Set initial texture rect to frame 0 so something visible appears
    // immediately even before the first draw() call
    this->sprite.setTextureRect(IntRect(5, 344, 21, 21));

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Tarma::~Tarma() {}
void Tarma::updateSprite() {
    // Animation is driven by PlayerSoldier::draw() via this->animation.
    // Nothing extra needed here for the basic walk — the function exists
    // to satisfy the pure-virtual contract and to be extended later
    // (e.g. shoot-while-walk frame override).
}
void Tarma::activatePowerUp() { this->immunityActive = true; this->immunityTimer.restart(); }
bool Tarma::hasVehicleSurvival() const { return true; }
void Tarma::onVehicleDestroyed() { this->exitVehicle(); }

void Tarma::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}




Eri::Eri(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , blastRadiusMultiplier(1.50f)
    , doubleGrenadeActive(false)
{
    // ── Load Eri's sprite sheet ──────────────────────────────────────────
    // File: resources/Sprites/Eri_Kasamoto.png   (background removed)
    // Walk strip: rows 289-329 of the sheet, 13 frames facing RIGHT
    Texture& tex = texMgr->getTexture("resources/Sprites/Eri_Kasamoto.png");

    // ── Configure walkAnim ────────────────────────────────────────────────
    // Frame sizes are NOT uniform (see w column below — ranges 19-31px).
    // That's normal for Metal Slug: each frame is hand-drawn individually.
    // useExplicitFrames will be true after the first setFrameRect() call,
    // so Animation::applyToSprite() uses our rects directly.
    this->walkAnim.setTexture(&tex);
    this->walkAnim.setFrameCount(13);
    this->walkAnim.setFrameDelay(5);   // 80ms per frame → ~12fps
    this->walkAnim.setLoop(true);

    this->walkAnim.setFrameRect( 0,   5, 292, 25, 34);
    this->walkAnim.setFrameRect( 1,  35, 292, 24, 34);
    this->walkAnim.setFrameRect( 2,  65, 292, 24, 34);
    this->walkAnim.setFrameRect( 3,  93, 292, 23, 34);
    this->walkAnim.setFrameRect( 4, 121, 291, 22, 35);
    this->walkAnim.setFrameRect( 5, 147, 291, 29, 35);
    this->walkAnim.setFrameRect( 6, 179, 293, 31, 33);
    this->walkAnim.setFrameRect( 7, 216, 292, 24, 34);
    this->walkAnim.setFrameRect( 8, 245, 293, 22, 33);
    this->walkAnim.setFrameRect( 9, 274, 292, 19, 34);
    this->walkAnim.setFrameRect(10, 299, 292, 20, 34);
    this->walkAnim.setFrameRect(11, 323, 292, 21, 34);
    this->walkAnim.setFrameRect(12, 349, 292, 24, 34);

    // Copy into base animation so Soldier's existing pipeline picks it up
    this->animation = this->walkAnim;

    // ── Sprite setup ─────────────────────────────────────────────────────
    // Scale chosen so on-screen height matches Marco:
    //   Marco baseline = 41px * 3.5  = 143.5px
    //   Eri   target   = 34px * 4.2f = 142.8px  ≈ same
    // physW/physH → hitbox height = 34*4.2 ≈ 143px, same as Marco → no teleport.
    this->physW = 25;
    this->physH = 34;

    this->sprite.setTexture(tex);
    this->sprite.setScale(4.2f, 4.2f);
    this->sprite.setTextureRect(IntRect(5, 292, 25, 34));  // frame 0

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Eri::~Eri() {}
void Eri::updateSprite() {
    // Driven by PlayerSoldier::draw() — no extra logic needed for walk-only.
    // Extend here later for shoot/grenade frame overrides.
}
void Eri::activatePowerUp() { this->doubleGrenadeActive = true; this->doubleGrenadeTimer.restart(); }

void Eri::meleeAttack() {
    Soldier::meleeAttack();
}

void Eri::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}





Fio::Fio(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , ammoBonusMultiplier(1.50f)
    , fireRateMultiplier(1.10f)
    , superchargedActive(false)
{
    // ── Load Fio's sprite sheet ──────────────────────────────────────────
    // File: resources/Sprites/Fiolina_Germi.png   (background removed)
    // Walk strip: rows 140-159 of the sheet, 16 frames facing RIGHT
    Texture& tex = texMgr->getTexture("resources/Sprites/Fiolina_Germi.png");

    // ── Configure walkAnim ────────────────────────────────────────────────
    // Fio's frames are small (~13x18px).  At 5x scale that's ~65x90px
    // on screen — a visible, gameplay-appropriate size.
    this->walkAnim.setTexture(&tex);
    this->walkAnim.setFrameCount(16);
    this->walkAnim.setFrameDelay(5);   // 80ms per frame → ~12fps
    this->walkAnim.setLoop(true);

    this->walkAnim.setFrameRect( 0,   4, 142, 14, 18);
    this->walkAnim.setFrameRect( 1,  21, 142, 13, 18);
    this->walkAnim.setFrameRect( 2,  36, 142, 11, 18);
    this->walkAnim.setFrameRect( 3,  50, 142, 12, 18);
    this->walkAnim.setFrameRect( 4,  64, 142, 12, 18);
    this->walkAnim.setFrameRect( 5,  78, 141, 12, 19);
    this->walkAnim.setFrameRect( 6,  93, 141, 12, 19);
    this->walkAnim.setFrameRect( 7, 107, 141, 11, 19);
    this->walkAnim.setFrameRect( 8, 121, 140, 11, 20);
    this->walkAnim.setFrameRect( 9, 134, 143, 15, 17);
    this->walkAnim.setFrameRect(10, 152, 143, 16, 17);
    this->walkAnim.setFrameRect(11, 170, 144, 16, 16);
    this->walkAnim.setFrameRect(12, 188, 144, 14, 16);
    this->walkAnim.setFrameRect(13, 204, 144, 13, 16);
    this->walkAnim.setFrameRect(14, 219, 143, 11, 17);
    this->walkAnim.setFrameRect(15, 232, 143, 10, 17);

    // Copy into base animation so Soldier's draw pipeline picks it up
    this->animation = this->walkAnim;

    // ── Sprite setup ─────────────────────────────────────────────────────
    // Scale chosen so on-screen height matches Marco:
    //   Marco baseline = 41px * 3.5  = 143.5px
    //   Fio   target   = 18px * 8.0f = 144.0px  ≈ same
    // physW/physH → hitbox height = 18*8.0 = 144px, same as Marco → no teleport.
    this->physW = 14;
    this->physH = 18;

    this->sprite.setTexture(tex);
    this->sprite.setScale(8.0f, 8.0f);
    this->sprite.setTextureRect(IntRect(4, 142, 14, 18));  // frame 0

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Fio::~Fio() {}
void Fio::updateSprite() {
    // Driven by PlayerSoldier::draw() — no extra logic needed for walk-only.
}
void Fio::activatePowerUp() { this->superchargedActive = true; this->superchargedTimer.restart(); }

void Fio::switchWeapon(Weapon* w) {
    // Fio gets 50% more ammo on pickup via ammoBonusMultiplier — hook here later.
    // For now delegate to base class behaviour.
    PlayerSoldier::switchWeapon(w);
}

void Fio::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}













void PlayerSoldier::throwGrenade() {
    if (this->pm == nullptr)          return;
    if (this->grenadeCount <= 0)      return;

    this->grenadeCount--;




    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->direction,
        36.f,
        24.f
    );



    const float LOB_ANGLE = 45.f;
    const int   NADE_DAMAGE = 20;
    const int   BLAST_RADIUS = 3;

    this->pm->spawnExplosive(origin, this->direction,
        LOB_ANGLE, NADE_DAMAGE,
        BLAST_RADIUS, false);
}




void Eri::throwGrenade() {
    if (this->pm == nullptr)     return;
    if (this->grenadeCount <= 0) return;


    this->grenadeCount--;

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position, this->direction, 36.f, 24.f
    );

    this->pm->spawnExplosive(origin, this->direction,
        45.f, 20, 3, false);



    if (this->doubleGrenadeActive && this->grenadeCount >= 1) {
        this->grenadeCount--;




        this->pm->spawnExplosive(origin, this->direction,
            30.f, 20, 3, false);

        if (this->doubleGrenadeTimer.getElapsedTime().asSeconds() >= 10.f)
            this->doubleGrenadeActive = false;
    }
}