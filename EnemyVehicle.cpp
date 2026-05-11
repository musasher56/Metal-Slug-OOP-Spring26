#include "EnemyVehicle.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "ProjectileManager.h"
#include <cmath>
#include <cstdlib>

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
            int worldOffX = lvl->getWorldOffX();
            int row = static_cast<int>(bottom) / cellSize;
            int col = static_cast<int>(this->position.x + 160.f * scaleX) / cellSize - worldOffX;

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

// ============================================================
// Submarine
// ============================================================

Submarine::Submarine(TextureManager* texMgr, AudioManager* audMgr)
    : EnemyVehicle(texMgr, audMgr)
    , subState(0)
    , swimSpeed(2.f)
    , patrolLeftX(10242.f)
    , patrolRightX(115919.f)
    , bombFired(false)
    , sinkVY(0.f)
    , deathDuration(3.0f)
{
    this->health = 8;
    this->maxHealth = 8;
    this->scoreValue = 500;
    this->biome = BIOME_AQUATIC;

    // Swim animation — submarine.png (7 frames)
    Texture& subTex = texMgr->getTexture("resources/Sprites/submarine.png");
    this->swimAnim.setTexture(&subTex);
    this->swimAnim.setFrameCount(7);
    this->swimAnim.setFrameDelay(8);
    this->swimAnim.setFrameRect(0, 0, 0, 372, 196);
    this->swimAnim.setFrameRect(1, 408, 4, 372, 192);
    this->swimAnim.setFrameRect(2, 816, 4, 368, 192);
    this->swimAnim.setFrameRect(3, 1220, 0, 368, 196);
    this->swimAnim.setFrameRect(4, 1620, 0, 372, 196);
    this->swimAnim.setFrameRect(5, 2024, 4, 372, 192);
    this->swimAnim.setFrameRect(6, 2428, 12, 372, 184);
    this->swimAnim.setLoop(true);

    // Base anim uses the same texture for bounding box reference
    this->anim.setTexture(&subTex);
    this->anim.setFrameCount(7);
    this->anim.setFrameDelay(8);
    this->anim.setFrameRect(0, 0, 0, 372, 196);
    this->anim.setFrameRect(1, 408, 4, 372, 192);
    this->anim.setFrameRect(2, 816, 4, 368, 192);
    this->anim.setFrameRect(3, 1220, 0, 368, 196);
    this->anim.setFrameRect(4, 1620, 0, 372, 196);
    this->anim.setFrameRect(5, 2024, 4, 372, 192);
    this->anim.setFrameRect(6, 2428, 12, 372, 184);
    this->anim.setLoop(true);

    this->sprite.setTexture(subTex);
    this->sprite.setTextureRect(IntRect(0, 0, 372, 196));
    this->sprite.setScale(0.8f, 0.8f);
    this->currentAnim = &this->swimAnim;
    this->updateBoundingBox();
}

Submarine::~Submarine() {}

void Submarine::setPatrolBounds(float leftX, float rightX) {
    this->patrolLeftX = leftX;
    this->patrolRightX = rightX;
}

void Submarine::setSwimDirection(int dir) {
    if (dir == DIR_RIGHT) {
        this->faceRight = true;
        this->velocityX = this->swimSpeed;
    }
    else {
        this->faceRight = false;
        this->velocityX = -this->swimSpeed;
    }
}

void Submarine::update(PlayerSoldier* player, ProjectileManager* projMgr,
    float scroll, Level* lvl)
{
    (void)lvl;
    if (!this->status) return;

    if (this->destroyed && this->subState == 0) {
        this->subState = 1;
    }

    if (this->subState == 0) {
        // ── SWIMMING: patrol back and forth in water, fire bomb at player ──
        this->position.x += this->velocityX;

        // Clamp to water rectangle: (10242,574) to (115919,1776)
        float subW = 372.f * 0.8f;  // sprite width * scale
        float subH = 196.f * 0.8f;  // sprite height * scale
        if (this->position.x < 10242.f) {
            this->position.x = 10242.f;
            this->setSwimDirection(DIR_RIGHT);
        }
        if (this->position.x + subW > 115919.f) {
            this->position.x = 115919.f - subW;
            this->setSwimDirection(DIR_LEFT);
        }
        if (this->position.y < 574.f) this->position.y = 574.f;
        if (this->position.y + subH > 1776.f) this->position.y = 1776.f - subH;

        // Reverse direction at patrol boundaries
        if (this->position.x <= this->patrolLeftX) {
            this->setSwimDirection(DIR_RIGHT);
        }
        else if (this->position.x >= this->patrolRightX) {
            this->setSwimDirection(DIR_LEFT);
        }

        // Fire bomb at player — rough aim with spread, not perfect tracking
        if (!this->bombFired && player != nullptr && projMgr != nullptr) {
            float dx = player->getPosition().x - this->position.x;
            float dy = player->getPosition().y - this->position.y;
            float dist = sqrtf(dx * dx + dy * dy);

            // Only fire if player is within ~600px range
            if (dist < 600.f && dist > 50.f) {
                // Calculate angle toward player with random spread (+/- 15 degrees)
                float angle = atan2f(-dy, dx) * 180.f / 3.14159f;
                float spread = ((rand() % 30) - 15);  // -15 to +15 degrees
                angle += spread;

                // Determine direction based on which side player is on
                int bombDir = (dx >= 0.f) ? DIR_RIGHT : DIR_LEFT;

                float scaleX = std::abs(this->sprite.getScale().x);
                float bombX = this->position.x + 186.f * scaleX;
                float bombY = this->position.y + 98.f;

                projMgr->spawnBomb(
                    sf::Vector2f(bombX, bombY),
                    bombDir, angle, 1, 3, true, 6.f);

                this->bombFired = true;
                this->bombCooldown.restart();
            }
        }

        // Reset bomb after cooldown (2.5 seconds)
        if (this->bombFired && this->bombCooldown.getElapsedTime().asSeconds() >= 2.5f) {
            this->bombFired = false;
        }

        this->currentAnim = &this->swimAnim;
    }
    else {
        // ── SINKING: slowly drift down ──
        this->sinkVY += 0.05f;
        this->position.y += this->sinkVY;

        // Safety timeout
        if (this->deathClock.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }

        this->currentAnim = &this->swimAnim;
    }
}

void Submarine::onDeath() {
    if (this->destroyed) return;
    this->destroyed = true;
    this->subState = 1;
    this->sinkVY = 0.5f;
    this->deathClock.restart();
}

void Submarine::updateBoundingBox() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    int boxW = static_cast<int>(372.f * scaleX);
    int boxH = static_cast<int>(196.f * scaleY);
    int boxX = 0;
    int boxY = 0;
    if (this->faceRight) {
        boxX = -boxW;
    }
    this->boundingBox = IntRect(boxX, boxY, boxW, boxH);
}

void Submarine::draw(RenderWindow& window, float scrollX, float scrollY) {
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

    // When sinking, slight tilt
    if (this->subState == 1) {
        float angle = this->faceRight ? 15.f : -15.f;
        this->sprite.setRotation(angle);
    }
    else {
        this->sprite.setRotation(0.f);
    }

    this->sprite.setPosition(this->position.x - scrollX,
        this->position.y - scrollY);
    window.draw(this->sprite);
}