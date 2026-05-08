#include "PlayerSoldier.h"
#include <iostream>
#include <fstream>




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
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    int w = static_cast<int>(36 * scaleX);
    int h = static_cast<int>(41 * scaleY);
    int left = 0;
    if (this->direction == DIR_LEFT) {
        left = -w;
    }
    this->boundingBox = IntRect(left, 0, w, h);
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








    if (Keyboard::isKeyPressed(Keyboard::Z)) {
        this->shoot();
    }



    if (this->dualFireActive && Keyboard::isKeyPressed(Keyboard::Z)) {
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


    if (Keyboard::isKeyPressed(Keyboard::X)) {
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
void Tarma::updateSprite() { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Tarma::activatePowerUp() { this->immunityActive = true; this->immunityTimer.restart(); }
bool Tarma::hasVehicleSurvival() const { return true; }
void Tarma::onVehicleDestroyed() { this->exitVehicle(); }

void Tarma::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
}




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
void Eri::updateSprite() { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Eri::activatePowerUp() { this->doubleGrenadeActive = true; this->doubleGrenadeTimer.restart(); }

void Eri::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
}





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
void Fio::updateSprite() { this->sprite.setTextureRect(IntRect(0, 0, 32, 32)); }
void Fio::activatePowerUp() { this->superchargedActive = true; this->superchargedTimer.restart(); }
void Fio::pickUpWeapon() {}

void Fio::handleInput() {

    if (Keyboard::isKeyPressed(Keyboard::Z)) this->shoot();
    if (Keyboard::isKeyPressed(Keyboard::X)) this->throwGrenade();
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