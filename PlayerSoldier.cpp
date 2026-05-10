#include "PlayerSoldier.h"
#include <iostream>
#include <fstream>
#include <cmath>


// ─────────────────────────────────────────────────────────────────────────────
// PlayerSoldier — base
// ─────────────────────────────────────────────────────────────────────────────

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
    , devWeaponIdx(0)
    , qWasPressed(false)
{
    for (int i = 0; i < 3; ++i) this->inventory[i] = nullptr;
    for (int i = 0; i < 5; ++i) this->devWeaponPool[i] = nullptr;

    this->pistol = new Pistol();
    this->devWeaponPool[0] = this->pistol;
    this->currentWeapon = this->pistol;

    this->devWeaponPool[1] = new HeavyMachineGun();
    this->devWeaponPool[2] = new RocketLauncher();
    this->devWeaponPool[3] = new FlameShot();
    this->devWeaponPool[4] = new LaserGun();
}

PlayerSoldier::~PlayerSoldier() {
    // currentWeapon always points into devWeaponPool — null it to prevent
    // any stale delete in code that checks currentWeapon != nullptr.
    this->currentWeapon = nullptr;

    if (this->pistol != nullptr) {
        delete this->pistol;
        this->pistol = nullptr;
        this->devWeaponPool[0] = nullptr;
    }
    for (int i = 1; i < 5; ++i) {
        if (this->devWeaponPool[i] != nullptr) {
            delete this->devWeaponPool[i];
            this->devWeaponPool[i] = nullptr;
        }
    }
    if (this->currentGrenade != nullptr) {
        delete this->currentGrenade;
        this->currentGrenade = nullptr;
    }
    for (int i = 0; i < this->inventorySize; ++i) {
        if (this->inventory[i] != nullptr) {
            delete this->inventory[i];
            this->inventory[i] = nullptr;
        }
    }
}

void PlayerSoldier::cycleWeapon() {
    this->devWeaponIdx = (this->devWeaponIdx + 1) % 5;
    this->currentWeapon = this->devWeaponPool[this->devWeaponIdx];
}

const char* PlayerSoldier::getCurrentWeaponName() const {
    static const char* names[5] = {
        "Pistol", "Heavy Machine Gun", "Rocket Launcher", "Flame Shot", "Laser Gun"
    };
    if (this->devWeaponIdx >= 0 && this->devWeaponIdx < 5)
        return names[this->devWeaponIdx];
    return "Unknown";
}

void PlayerSoldier::setProjectileManager(ProjectileManager* manager) {
    this->pm = manager;
}

void PlayerSoldier::updateAim(sf::Vector2f mousePos) {
    this->aimController.update(mousePos, this->position, this->direction);
}

void PlayerSoldier::shoot() {
    if (this->pm == nullptr)             return;
    if (this->currentWeapon == nullptr)  return;
    if (!this->currentWeapon->hasAmmo()) return;

    float angle = this->aimController.getAngle();

    // Use the character's actual physics dimensions (physW * scale) for
    // barrel positioning instead of hardcoded 60/52 which was tuned for
    // Marco only.  This ensures all characters (Marco, Tarma, Eri, Fio)
    // fire from the correct barrel tip position.
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float spriteW = (float)this->physW * scaleX;
    float spriteH = (float)this->physH * scaleY;

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position, this->direction, spriteW, spriteH * 0.36f);
    this->currentWeapon->fire(origin, this->direction, angle, this->pm);
}

void PlayerSoldier::meleeAttack() {
    if (this->pm == nullptr) return;

    // Cooldown gate — meleeCooldown is 0.5s by default
    if (this->meleeTimer.getElapsedTime().asSeconds() < this->meleeCooldown) return;
    this->meleeTimer.restart();

    // Position the melee slash in front of the player.
    // The slash origin is the anchor point; MeleeSlash::getBoundingBox()
    // extends the hitbox in slashDir from that anchor.
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float spriteW = (float)this->physW * scaleX;
    float spriteH = (float)this->physH * scaleY;

    // X: place the slash anchor at the player's front edge so the hitbox
    // extends OUTWARD into enemy territory, not inside the player's body.
    //
    //  DIR_RIGHT: front edge = position.x + spriteW
    //    MeleeSlash (DIR_RIGHT) hitbox: [anchor, anchor + slashWidth]
    //    So anchor at the front edge means the slash extends rightward from
    //    the player's right side — exactly where enemies would be.
    //
    //  DIR_LEFT:  front edge = position.x
    //    MeleeSlash (DIR_LEFT) hitbox: [anchor - slashWidth, anchor]
    //    So anchor at the front edge means the slash extends leftward from
    //    the player's left side — exactly where enemies would be.
    //
    //  Previously both directions used position.x + spriteW*0.5 (center),
    //  which for DIR_LEFT placed the slash partially behind the player.
    float slashX;
    if (this->direction == DIR_RIGHT) {
        slashX = this->position.x + spriteW;       // front (right) edge
    } else {
        slashX = this->position.x;                  // front (left) edge
    }

    // Y: use HEIGHT to offset to chest level — roughly 25% down
    // from the top of the sprite puts the slash at chest/shoulder height.
    float slashY = this->position.y + spriteH * 0.25f;

    this->pm->spawnMelee(sf::Vector2f(slashX, slashY), this->direction, 3, false);
}

void PlayerSoldier::switchWeapon(Weapon* w) {
    if (w == nullptr) return;

    bool isPoolWeapon = false;
    for (int i = 0; i < 5; ++i) {
        if (this->currentWeapon == this->devWeaponPool[i]) { isPoolWeapon = true; break; }
    }
    if (!isPoolWeapon && this->currentWeapon != nullptr && this->inventorySize < 3)
        this->inventory[this->inventorySize++] = this->currentWeapon;

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
    if (amount < 0)         return;
    if (this->isInvincible) return;
    if (this->transformState != nullptr) {}

    this->enemyBulletHits += amount;
    while (this->enemyBulletHits >= HITS_PER_HEART) {
        this->enemyBulletHits -= HITS_PER_HEART;
        this->currentHP -= 1;
    }
    if (this->currentHP < 0) this->currentHP = 0;
    if (this->currentHP == 0) this->onDeath();
    this->health = this->currentHP;
}

void PlayerSoldier::updateBoundingBox() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    int w = static_cast<int>(this->physW * scaleX);
    int h = static_cast<int>(this->physH * scaleY);
    int left = (this->direction == DIR_LEFT) ? -w : 0;
    this->boundingBox = IntRect(left, 0, w, h);
}

void PlayerSoldier::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (this->velocityX != 0.f || !this->onGround)
        this->animation.update();
    else {
        this->animation.currentFrame = 0;
        this->animation.clock.restart();
    }

    this->animation.applyToSprite(this->sprite);

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    this->sprite.setScale(
        (this->direction == DIR_RIGHT) ? scaleX : -scaleX,
        scaleY
    );
    this->sprite.setPosition(this->position.x - scrollX,
        this->position.y - scrollY);
    window.draw(this->sprite);
}


// ─────────────────────────────────────────────────────────────────────────────
// Marco  (existing sprite sheet — unchanged)
// ─────────────────────────────────────────────────────────────────────────────

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
    this->physW = 36;
    this->physH = 41;
    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
    if (texMgr != nullptr) {
        texMgr->loadTextureWithMask("marco-machinegun",
            "resources/Sprites/marco-machinegun.png", sf::Color::Black, 40);
        texMgr->loadTextureWithMask("marco-laser",
            "resources/Sprites/marco-laser.png",      sf::Color::Black, 40);
        texMgr->loadTextureWithMask("marco-flame",
            "resources/Sprites/marco-flame.png",      sf::Color::Black, 40);
        texMgr->loadTextureWithMask("marco-rocket",
            "resources/Sprites/marco-rocket.png",     sf::Color::Black, 40);

        // Each PNG: 1024x1536, 6 rows of 1024x256 strips.
        // Rows 1-4 are active walk frames (row 0 and 5 are empty).
        auto setupWpnAnim = [&](Animation& anim, const char* key) {
            Texture& tex = texMgr->getTexture(key);
            anim.setTexture(&tex);
            anim.setFrameCount(4);
            anim.setFrameDelay(6);
            anim.setLoop(true);
            anim.useExplicitFrames = true;
            anim.setFrameRect(0, 0, 256,  1024, 256);
            anim.setFrameRect(1, 0, 512,  1024, 256);
            anim.setFrameRect(2, 0, 768,  1024, 256);
            anim.setFrameRect(3, 0, 1024, 1024, 256);
        };
        setupWpnAnim(this->hmgAnim,    "marco-machinegun");
        setupWpnAnim(this->laserAnim,  "marco-laser");
        setupWpnAnim(this->flameAnim,  "marco-flame");
        setupWpnAnim(this->rocketAnim, "marco-rocket");
    }
    this->usingWeaponSpr = false;
    this->lastWeaponType = WEAPON_PISTOL;  // start with pistol, track for switch detection
}


Marco::~Marco() {}
void Marco::updateSprite() {
    this->sprite.setTextureRect(IntRect(0, 0, 36, 41));
    this->usingWeaponSpr = false;

    if (this->currentWeapon) {
        int wtype = this->currentWeapon->getType();
        Animation* wAnim = nullptr;
        if      (wtype == WEAPON_HMG)             wAnim = &this->hmgAnim;
        else if (wtype == WEAPON_LASER_GUN)        wAnim = &this->laserAnim;
        else if (wtype == WEAPON_FLAME_SHOT)       wAnim = &this->flameAnim;
        else if (wtype == WEAPON_ROCKET_LAUNCHER)  wAnim = &this->rocketAnim;

        // ── Reset animation on weapon switch ──────────────────────────────
        // When the player presses Q to cycle weapons, the new weapon's
        // animation must start from frame 0.  Without this reset, the new
        // weapon's animation continues from wherever it was last left,
        // causing a visual glitch (wrong frame briefly shown, then snapping
        // to the correct sequence).  The reset ensures a clean transition.
        if (wtype != this->lastWeaponType) {
            if (wAnim != nullptr) {
                wAnim->reset();
            }
            // Also reset the OLD weapon's animation so it starts fresh next time
            Animation* oldAnim = nullptr;
            if      (this->lastWeaponType == WEAPON_HMG)             oldAnim = &this->hmgAnim;
            else if (this->lastWeaponType == WEAPON_LASER_GUN)        oldAnim = &this->laserAnim;
            else if (this->lastWeaponType == WEAPON_FLAME_SHOT)       oldAnim = &this->flameAnim;
            else if (this->lastWeaponType == WEAPON_ROCKET_LAUNCHER)  oldAnim = &this->rocketAnim;
            if (oldAnim != nullptr) {
                oldAnim->reset();
            }
            this->lastWeaponType = wtype;
        }

        if (wAnim != nullptr) {
            wAnim->update();
            wAnim->applyToSprite(this->weaponSpr);
            float signX = (this->direction == DIR_RIGHT) ? 1.f : -1.f;
            // 0.56f: 256px frame height * 0.56 = 143px (matches physH*scale)
            this->weaponSpr.setScale(signX * 0.56f, 0.56f);
            this->usingWeaponSpr = true;
        }
    } else {
        this->lastWeaponType = WEAPON_PISTOL;
    }
}
void Marco::activatePowerUp() { this->dualFireActive = true; this->dualFireTimer.restart(); }

void Marco::handleInput() {
    bool qNow = Keyboard::isKeyPressed(Keyboard::Q);
    if (qNow && !this->qWasPressed) this->cycleWeapon();
    this->qWasPressed = qNow;

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::V)) this->meleeAttack();

    if (this->dualFireActive && Keyboard::isKeyPressed(Keyboard::X)) {
        if (this->pm && this->currentWeapon) {
            int opp = (this->direction == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
            sf::Vector2f o = ProjectileManager::calcBarrelTip(
                this->position, opp, 36.f, 20.f);
            this->currentWeapon->fire(o, opp, this->aimController.getAngle(), this->pm);
        }
        if (this->dualFireTimer.getElapsedTime().asSeconds() >= 10.f)
            this->dualFireActive = false;
    }

    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}

void Marco::meleeAttack() { PlayerSoldier::meleeAttack(); }

void Marco::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    // MUST call updateSprite() every frame so the weapon sprite changes
    // when the player cycles weapons with Q.  Without this call, usingWeaponSpr
    // stays false and only the default pistol sprite ever renders.
    this->updateSprite();

    float drawX = this->position.x - scrollX;
    float drawY = this->position.y - scrollY;

    if (this->usingWeaponSpr) {
        // Weapon sprite height (256 * 0.56 = 143px) matches physics feet position.
        // When facing LEFT the negative scale flips the sprite around its origin
        // (top-left corner by default), which shifts it left by one full frame
        // width.  We compensate by adding the frame width back so the sprite
        // stays aligned with the character's feet regardless of facing direction.
        float frameW = 256.f * 0.56f;  // weapon frame width in screen pixels
        if (this->direction == DIR_LEFT) {
            this->weaponSpr.setPosition(drawX + frameW, drawY);
        } else {
            this->weaponSpr.setPosition(drawX, drawY);
        }
        window.draw(this->weaponSpr);
    } else {
        // Default: draw physics sprite (your existing texture, unchanged)
        this->sprite.setPosition(drawX, drawY);
        window.draw(this->sprite);
    }

    // Draw grenade if in flight
    if (this->currentGrenade && this->currentGrenade->isActive()) {
        this->currentGrenade->draw(window, scrollX, scrollY);
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Tarma
//
// Sprite  : resources/Sprites/tarma.png  (1024×1024, RGB black background)
// Crop    : (47, 64, 913, 896) — user-specified
// Scale   : 0.2f  — user-specified
//   On-screen: 182 × 179 px
//
// WHY loadTextureWithMask is assigned directly (no getTexture call):
//   The previous pattern of loadTextureWithMask() → getTexture() required a
//   second findSlot() lookup that sometimes failed to locate the just-stored
//   key (visible in log as "[WARN] Texture not found: tarma-idle").
//   loadTextureWithMask now returns Texture& directly — zero lookup risk.
// ─────────────────────────────────────────────────────────────────────────────

Tarma::Tarma(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , vehicleFireRateBonus(0.25f)
    , vehicleDurabilityBonus(0.20f)
    , immunityActive(false)
{
    // loadTextureWithMask returns Texture& directly — no second lookup needed.
    Texture& tex = texMgr->loadTextureWithMask(
        "tarma-idle",
        "resources/Sprites/tarma.png",
        sf::Color::Black, 50
    );

    // Single idle/shoot frame — setFrameRect sets useExplicitFrames = true
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(1);
    this->animation.setFrameRect(0, 47, 64, 913, 896);
    this->animation.setLoop(true);

    // physW at 60 % of crop to exclude gun barrel from hitbox:
    //   913 * 0.6 = 547;  547 * 0.2 = 109 px effective hitbox width
    // physH = full crop:  896 * 0.2 = 179 px height
    this->physW = static_cast<int>(913 * 0.6f);
    this->physH = 896;

    this->sprite.setTexture(tex);
    this->sprite.setTextureRect(IntRect(47, 64, 913, 896));
    this->sprite.setScale(0.2f, 0.2f);

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Tarma::~Tarma() {}
void Tarma::updateSprite() {}
void Tarma::activatePowerUp() { this->immunityActive = true; this->immunityTimer.restart(); }
bool Tarma::hasVehicleSurvival() const { return true; }
void Tarma::onVehicleDestroyed() { this->exitVehicle(); }

void Tarma::handleInput() {
    bool qNow = Keyboard::isKeyPressed(Keyboard::Q);
    if (qNow && !this->qWasPressed) this->cycleWeapon();
    this->qWasPressed = qNow;

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::V)) this->meleeAttack();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}


// ─────────────────────────────────────────────────────────────────────────────
// Eri
//
// Sprite  : resources/Sprites/eri-idle.png  (300×258, RGB black background)
// Crop    : (13, 0, 287, 258) — pixel-scanned, exact match with user spec
// Scale   : 0.55f → 258 * 0.55 = 142 px  ≈  Marco's 41 * 3.5 = 143 px
// ─────────────────────────────────────────────────────────────────────────────

Eri::Eri(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , blastRadiusMultiplier(1.50f)
    , doubleGrenadeActive(false)
{
    Texture& tex = texMgr->loadTextureWithMask(
        "eri-idle",
        "resources/Sprites/eri-idle.png",
        sf::Color::Black, 50
    );

    this->animation.setTexture(&tex);
    this->animation.setFrameCount(1);
    this->animation.setFrameRect(0, 13, 0, 287, 258);
    this->animation.setLoop(true);

    // physW uses the FULL crop width (287), same pattern as Fio (physW=304).
    // The previous value (287*0.55=157) had the scale baked in, causing
    // updateBoundingBox() to double-apply the scale: 157*0.55=87 px, which
    // was far too narrow compared to Marco's 36*3.5=126 px.  With physW=287,
    // the box becomes 287*0.55=158 px — consistent with other characters.
    this->physW = 287;
    this->physH = 258;

    this->sprite.setTexture(tex);
    this->sprite.setTextureRect(IntRect(13, 0, 287, 258));
    this->sprite.setScale(0.55f, 0.55f);

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Eri::~Eri() {}
void Eri::updateSprite() {}
void Eri::activatePowerUp() { this->doubleGrenadeActive = true; this->doubleGrenadeTimer.restart(); }
void Eri::meleeAttack() { PlayerSoldier::meleeAttack(); }

void Eri::handleInput() {
    bool qNow = Keyboard::isKeyPressed(Keyboard::Q);
    if (qNow && !this->qWasPressed) this->cycleWeapon();
    this->qWasPressed = qNow;

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::V)) this->meleeAttack();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}

void Eri::throwGrenade() {
    if (this->pm == nullptr)     return;
    if (this->grenadeCount <= 0) return;
    this->grenadeCount--;

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position, this->direction, 36.f, 24.f);
    this->pm->spawnExplosive(origin, this->direction, 45.f, 20, 3, false);

    if (this->doubleGrenadeActive && this->grenadeCount >= 1) {
        this->grenadeCount--;
        this->pm->spawnExplosive(origin, this->direction, 30.f, 20, 3, false);
        if (this->doubleGrenadeTimer.getElapsedTime().asSeconds() >= 10.f)
            this->doubleGrenadeActive = false;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Fio (Germi)
//
// Sprite  : resources/Sprites/germi-idle.png  (334×264, RGB black background)
// Crop    : (25, 0, 304, 260) — measured by pixel scan
// Scale   : 0.55f → 260 * 0.55 = 143 px  ←  matches Marco exactly
// ─────────────────────────────────────────────────────────────────────────────

Fio::Fio(TextureManager* texMgr, AudioManager* audMgr)
    : PlayerSoldier(texMgr, audMgr)
    , ammoBonusMultiplier(1.50f)
    , fireRateMultiplier(1.10f)
    , superchargedActive(false)
{
    Texture& tex = texMgr->loadTextureWithMask(
        "germi-idle",
        "resources/Sprites/germi-idle.png",
        sf::Color::Black, 50
    );

    this->animation.setTexture(&tex);
    this->animation.setFrameCount(1);
    this->animation.setFrameRect(0, 25, 0, 304, 260);
    this->animation.setLoop(true);

    this->physW = 304;
    this->physH = 260;

    this->sprite.setTexture(tex);
    this->sprite.setTextureRect(IntRect(25, 0, 304, 260));
    this->sprite.setScale(0.55f, 0.55f);

    this->position = sf::Vector2f(200.f, 300.f);
    this->updateBoundingBox();
}

Fio::~Fio() {}
void Fio::updateSprite() {}
void Fio::activatePowerUp() { this->superchargedActive = true; this->superchargedTimer.restart(); }

void Fio::switchWeapon(Weapon* w) {
    PlayerSoldier::switchWeapon(w);
}

void Fio::handleInput() {
    bool qNow = Keyboard::isKeyPressed(Keyboard::Q);
    if (qNow && !this->qWasPressed) this->cycleWeapon();
    this->qWasPressed = qNow;

    if (Keyboard::isKeyPressed(Keyboard::X)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::V)) this->meleeAttack();
    if (Keyboard::isKeyPressed(Keyboard::C)) this->throwGrenade();
}


// ─────────────────────────────────────────────────────────────────────────────
// PlayerSoldier::throwGrenade  (base — Marco, Tarma)
// ─────────────────────────────────────────────────────────────────────────────

void PlayerSoldier::throwGrenade() {
    if (this->pm == nullptr)     return;
    if (this->grenadeCount <= 0) return;
    this->grenadeCount--;

    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position, this->direction, 36.f, 24.f);
    this->pm->spawnExplosive(origin, this->direction, 45.f, 20, 3, false);
}