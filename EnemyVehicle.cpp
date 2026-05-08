#include "EnemyVehicle.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "ProjectileManager.h"
#include <cmath>

// ============================================================
// EnemyVehicle — base class
// ============================================================

EnemyVehicle::EnemyVehicle(TextureManager* texMgr, AudioManager* audMgr)
    : DamagableEntity(texMgr, audMgr)
    , biome(BIOME_AERIAL)
    , pm(nullptr)
    , destroyed(false)
    , faceRight(true)
    , velocityX(0.f)
    , velocityY(0.f)
    , currentAnim(nullptr)
{
    this->health = 5;
    this->maxHealth = 5;
    this->scoreValue = 300;
}

EnemyVehicle::~EnemyVehicle() {}

void EnemyVehicle::setProjectileManager(ProjectileManager* p) {
    this->pm = p;
}

bool EnemyVehicle::isDestroyed() const {
    return this->destroyed;
}

void EnemyVehicle::update(float scroll, Level* lvl) {
    (void)scroll; (void)lvl;
}

void EnemyVehicle::onDeath() {
    this->destroyed = true;
}

void EnemyVehicle::updateBoundingBox() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float drawW = (float)(this->anim.frames[0].width) * scaleX;
    float drawH = (float)(this->anim.frames[0].height) * scaleY;
    int boxW = static_cast<int>(drawW);
    int boxH = static_cast<int>(drawH);
    int boxX = 0;
    int boxY = 0;
    if (this->faceRight) {
        boxX = -boxW;
    }
    this->boundingBox = IntRect(boxX, boxY, boxW, boxH);
}

// ============================================================
// FlyingTara
// ============================================================

FlyingTara::FlyingTara(TextureManager* texMgr, AudioManager* audMgr)
    : EnemyVehicle(texMgr, audMgr)
    , taraState(0)
    , flySpeed(5.f)
    , bombDropped(false)
    , crashVX(0.f)
    , crashVY(0.f)
    , deathDuration(3.0f)
{
    this->health = 5;
    this->maxHealth = 5;
    this->scoreValue = 300;
    this->biome = BIOME_AERIAL;

    // Fly animation — flying-tara.png (4 frames)
    Texture& flyTex = texMgr->getTexture("resources/Sprites/flying-tara.png");
    this->flyAnim.setTexture(&flyTex);
    this->flyAnim.setFrameCount(4);
    this->flyAnim.setFrameDelay(6);
    this->flyAnim.setFrameRect(0, 0, 0, 320, 188);
    this->flyAnim.setFrameRect(1, 332, 4, 320, 180);
    this->flyAnim.setFrameRect(2, 664, 8, 320, 172);
    this->flyAnim.setFrameRect(3, 996, 12, 320, 164);
    this->flyAnim.setLoop(true);

    // Base anim also uses the fly texture (for bounding box reference)
    this->anim.setTexture(&flyTex);
    this->anim.setFrameCount(4);
    this->anim.setFrameDelay(6);
    this->anim.setFrameRect(0, 0, 0, 320, 188);
    this->anim.setFrameRect(1, 332, 4, 320, 180);
    this->anim.setFrameRect(2, 664, 8, 320, 172);
    this->anim.setFrameRect(3, 996, 12, 320, 164);
    this->anim.setLoop(true);

    this->sprite.setTexture(flyTex);
    this->sprite.setTextureRect(IntRect(0, 0, 320, 188));
    this->sprite.setScale(0.8f, 0.8f);
    this->currentAnim = &this->flyAnim;
    this->updateBoundingBox();
}

FlyingTara::~FlyingTara() {}

void FlyingTara::setFlyDirection(int dir) {
    if (dir == DIR_RIGHT) {
        this->faceRight = true;
        this->velocityX = this->flySpeed;
    }
    else {
        this->faceRight = false;
        this->velocityX = -this->flySpeed;
    }
}

void FlyingTara::update(PlayerSoldier* player, ProjectileManager* projMgr,
    float scroll, Level* lvl)
{
    if (!this->status) return;
    if (this->destroyed && this->taraState == 0) {
        this->taraState = 1;
    }

    if (this->taraState == 0) {
        // ── FLYING: move horizontally, drop bomb when over player ──
        this->position.x += this->velocityX;
        this->position.y += this->velocityY;

        if (!this->bombDropped && player != nullptr && projMgr != nullptr) {
            float dx = fabsf(player->getPosition().x - this->position.x);
            if (dx < 80.f) {
                float scaleX = std::abs(this->sprite.getScale().x);
                float scaleY = std::abs(this->sprite.getScale().y);
                float bombX = this->position.x + 160.f * scaleX;
                float bombY = this->position.y + 188.f * scaleY;

                int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
                projMgr->spawnBomb(
                    sf::Vector2f(bombX, bombY),
                    dir, 85.f, 3, 3, true);

                this->bombDropped = true;   // <-- THIS must be set BEFORE any other logic
            }
        }

        if (this->position.x < scroll - 1000.f ||
            this->position.x > scroll + (float)SCREEN_W + 1000.f) {
            this->status = false;
        }

        this->currentAnim = &this->flyAnim;
    }
    else {
        // ── CRASHING: arc down at angle until ground impact ──
        this->crashVY += 0.2f;
        this->velocityX = this->crashVX;
        this->velocityY = this->crashVY;

        this->position.x += this->velocityX;
        this->position.y += this->velocityY;

        if (lvl != nullptr) {
            float scaleX = std::abs(this->sprite.getScale().x);
            float scaleY = std::abs(this->sprite.getScale().y);
            float bottom = this->position.y + 188.f * scaleY;
            int cellSize = lvl->getCellSize();
            int row = static_cast<int>(bottom) / cellSize;
            int col = static_cast<int>(this->position.x + 160.f * scaleX) / cellSize;

            if (lvl->isSolid(row, col)) {
                if (projMgr != nullptr) {
                    projMgr->spawnBlast(
                        this->position.x + 160.f * scaleX,
                        bottom);
                }
                this->status = false;
                return;
            }
        }

        if (this->deathClock.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }

        this->currentAnim = &this->flyAnim;
    }
}

void FlyingTara::onDeath() {
    if (this->destroyed) return;
    this->destroyed = true;
    this->taraState = 1;

    this->crashVX = this->faceRight ? 2.0f : -2.0f;
    this->crashVY = -2.f;
    this->deathClock.restart();
}

void FlyingTara::updateBoundingBox() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    int boxW = static_cast<int>(320.f * scaleX);
    int boxH = static_cast<int>(188.f * scaleY);
    int boxX = 0;
    int boxY = 0;
    if (this->faceRight) {
        boxX = -boxW;
    }
    this->boundingBox = IntRect(boxX, boxY, boxW, boxH);
}

void FlyingTara::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    if (this->currentAnim != nullptr) {
        this->currentAnim->update();
        this->currentAnim->applyToSprite(this->sprite);
    }

    float scale = 0.8f;

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    if (this->taraState == 1) {
        float angle = this->faceRight ? 25.f : -25.f;
        this->sprite.setRotation(angle);
    }
    else {
        this->sprite.setRotation(0.f);
    }

    this->sprite.setPosition(this->position.x - scrollX,
        this->position.y - scrollY);
    window.draw(this->sprite);
}