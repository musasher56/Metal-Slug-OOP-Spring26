#include "Enemy.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(TextureManager* texMgr, AudioManager* audMgr)
    : Soldier(texMgr, audMgr)
    , aiState(AI_IDLE)
    , detectionRange(400.f)
    , attackRange(300.f)
    , attackCooldown(1.5f)
    , pm(nullptr)
    , activated(false)
    , patrolCenter(0.f)
    , patrolRadius(100.f)
    , enemyType(ENEMY_REBEL)
    , currentAnim(nullptr)
    , frameW(78)
    , frameH(66)
    , baseFrameW(78)
    , baseFrameH(66)
    , walkFrames(10)
    , shootFrames(8)
    , deathFrames(6)
    , dying(false)
    , deathDuration(1.0f)
    , deathSpriteScale(2.0f)
    , faceRight(false)
{
    this->maxVelocity = 2.f;
    this->baseMaxVelocity = 2.f;
    this->currentHP = 3;
    this->health = 3;
    this->maxHealth = 3;
    this->lives = 1;
    this->scoreValue = 100;

    this->walkAnim.setLoop(true);
    this->shootAnim.setLoop(false);
    this->deathAnim.setLoop(false);
}

Enemy::~Enemy() {}

void Enemy::setProjectileManager(ProjectileManager* manager) {
    this->pm = manager;
}

void Enemy::setPatrol(float centerX, float radius) {
    this->patrolCenter = centerX;
    this->patrolRadius = radius;
}

void Enemy::setEnemyType(int type) {
    this->enemyType = type;
}

bool Enemy::isActivated() const {
    return this->activated;
}

bool Enemy::isDying() const {
    return this->dying;
}

float Enemy::distanceTo(PlayerSoldier* player) const {
    if (player == nullptr) return 9999.f;
    float dx = player->getPosition().x - this->position.x;
    float dy = player->getPosition().y - this->position.y;
    return sqrtf(dx * dx + dy * dy);
}

bool Enemy::playerInRange(PlayerSoldier* player, float range) const {
    return this->distanceTo(player) <= range;
}

void Enemy::switchAnim(Animation* newAnim) {
    if (this->currentAnim == newAnim) return;
    if (this->currentAnim != nullptr) {
        this->currentAnim->reset();
    }
    this->currentAnim = newAnim;
    if (this->currentAnim != nullptr) {
        this->currentAnim->reset();
    }
}

void Enemy::applyDirectionFlip() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    if (this->faceRight) {
        this->sprite.setScale(-scaleX, scaleY);
    }
    else {
        this->sprite.setScale(scaleX, scaleY);
    }
}

void Enemy::updateAI(PlayerSoldier* player, Level* lvl) {
    (void)lvl;

    if (player == nullptr) return;

    if (this->dying) return;

    if (!this->activated) {
        if (this->playerInRange(player, this->detectionRange)) {
            this->activated = true;
            this->aiState = AI_CHASE;
        }
        this->switchAnim(&this->walkAnim);
        return;
    }

    if (this->currentHP <= 0) {
        this->aiState = AI_IDLE;
        return;
    }

    if (this->playerInRange(player, this->attackRange)) {
        this->aiState = AI_ATTACK;
        this->velocityX = 0.f;

        float px = player->getPosition().x;
        this->faceRight = (px > this->position.x);

        if (this->attackTimer.getElapsedTime().asSeconds() >= this->attackCooldown) {
            this->performAttack(player);
            this->attackTimer.restart();
            this->switchAnim(&this->shootAnim);
        }

        if (this->currentAnim == &this->shootAnim && this->shootAnim.isFinished()) {
            this->switchAnim(&this->walkAnim);
        }
    }
    else if (this->playerInRange(player, this->detectionRange)) {
        this->aiState = AI_CHASE;
        this->switchAnim(&this->walkAnim);

        float px = player->getPosition().x;
        this->faceRight = (px > this->position.x);

        if (px > this->position.x + 10.f) {
            this->velocityX += 0.3f;
            if (this->velocityX > this->maxVelocity)
                this->velocityX = this->maxVelocity;
        }
        else if (px < this->position.x - 10.f) {
            this->velocityX -= 0.3f;
            if (this->velocityX < -this->maxVelocity)
                this->velocityX = -this->maxVelocity;
        }
        else {
            this->decelerate();
        }
    }
    else {
        this->aiState = AI_PATROL;
        this->switchAnim(&this->walkAnim);

        float leftBound = this->patrolCenter - this->patrolRadius;
        float rightBound = this->patrolCenter + this->patrolRadius;

        if (this->position.x <= leftBound) {
            this->direction = DIR_RIGHT;
            this->faceRight = true;
            this->velocityX = this->maxVelocity * 0.5f;
        }
        else if (this->position.x >= rightBound) {
            this->direction = DIR_LEFT;
            this->faceRight = false;
            this->velocityX = -this->maxVelocity * 0.5f;
        }
        else {
            if (this->velocityX == 0.f) {
                int r = rand() % 100;
                if (r < 2) {
                    this->direction = DIR_RIGHT;
                    this->faceRight = true;
                    this->velocityX = this->maxVelocity * 0.5f;
                }
                else if (r < 4) {
                    this->direction = DIR_LEFT;
                    this->faceRight = false;
                    this->velocityX = -this->maxVelocity * 0.5f;
                }
            }
        }
    }
}

void Enemy::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.45f
    );

    float angle = 0.f;
    float dy = player->getPosition().y - this->position.y;
    float dx = player->getPosition().x - this->position.x;
    if (dx != 0.f || dy != 0.f) {
        angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
        if (angle < 0.f) angle = 0.f;
        if (angle > 45.f) angle = 45.f;
    }

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    this->pm->spawnStraight(origin, dir, angle, 1, true);
}

void Enemy::onDeath() {
    if (this->dying) return;
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->sprite.setScale(this->deathSpriteScale, this->deathSpriteScale);
    this->switchAnim(&this->deathAnim);
}

void Enemy::updateBoundingBox() {
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float drawW = (float)(this->baseFrameW) * scaleX;
    float drawH = (float)(this->baseFrameH) * scaleY;
    int boxW = static_cast<int>(drawW);
    int boxH = static_cast<int>(drawH);
    int boxX = 0;
    int boxY = 0;
    if (this->faceRight) {
        boxX = -boxW;
    }
    this->boundingBox = IntRect(boxX, boxY, boxW, boxH);
}

void Enemy::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
            return;
        }
    }

    if (this->currentAnim != nullptr) {
        this->currentAnim->update();
        this->currentAnim->applyToSprite(this->sprite);
    }

    this->applyDirectionFlip();

    float drawY = this->position.y - scrollY;
    if (this->dying) {
        drawY += 70.f;
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

void Enemy::handleCollision(Level* lvl) {
    if (lvl == nullptr) return;
    if (this->dying) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float enemyLeft = this->position.x;
    float enemyRight = this->position.x + (float)(this->baseFrameW) * scaleX;
    float enemyTop = this->position.y;
    float enemyBottom = this->position.y + (float)(this->baseFrameH) * scaleY;

    int cellSize = lvl->getCellSize();
    this->onGround = false;

    int startCol = static_cast<int>(enemyLeft) / cellSize - 1;
    int endCol = static_cast<int>(enemyRight) / cellSize + 1;
    int startRow = static_cast<int>(enemyTop) / cellSize - 1;
    int endRow = static_cast<int>(enemyBottom) / cellSize + 1;

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) continue;

            float blockLeft = static_cast<float>(col * cellSize);
            float blockRight = blockLeft + static_cast<float>(cellSize);
            float blockTop = static_cast<float>(row * cellSize);
            float blockBottom = blockTop + static_cast<float>(cellSize);

            if (enemyRight > blockLeft && enemyLeft < blockRight &&
                enemyBottom > blockTop && enemyTop < blockBottom) {

                float overlapLeft = enemyRight - blockLeft;
                float overlapRight = blockRight - enemyLeft;
                float overlapTop = enemyBottom - blockTop;
                float overlapBottom = blockBottom - enemyTop;

                float minOverlap = overlapLeft;
                int resolveDir = 0;

                if (overlapRight < minOverlap) {
                    minOverlap = overlapRight;
                    resolveDir = 2;
                }
                if (overlapTop < minOverlap) {
                    minOverlap = overlapTop;
                    resolveDir = 3;
                }
                if (overlapBottom < minOverlap) {
                    minOverlap = overlapBottom;
                    resolveDir = 4;
                }

                if (resolveDir == 1) {
                    this->position.x -= minOverlap;
                    this->velocityX = 0.f;
                }
                else if (resolveDir == 2) {
                    this->position.x += minOverlap;
                    this->velocityX = 0.f;
                }
                else if (resolveDir == 3) {
                    this->position.y -= minOverlap;
                    this->velocityY = 0.f;
                    this->onGround = true;
                }
                else if (resolveDir == 4) {
                    this->position.y += minOverlap;
                    this->velocityY = 0.f;
                }

                enemyLeft = this->position.x;
                enemyRight = this->position.x + (float)(this->baseFrameW) * scaleX;
                enemyTop = this->position.y;
                enemyBottom = this->position.y + (float)(this->baseFrameH) * scaleY;
            }
        }
    }

    if (!this->onGround) {
        float probeY = enemyBottom + 1.0f;
        int probeRow = static_cast<int>(probeY) / cellSize;
        int probeStartCol = static_cast<int>(enemyLeft + 2) / cellSize;
        int probeEndCol = static_cast<int>(enemyRight - 2) / cellSize;
        for (int col = probeStartCol; col <= probeEndCol; ++col) {
            if (lvl->isSolid(probeRow, col)) {
                this->onGround = true;
                break;
            }
        }
    }
}

int Enemy::getAIState() const { return this->aiState; }
int Enemy::getEnemyType() const { return this->enemyType; }

void Enemy::takeDamage(int amount) {
    if (this->dying) return;
    Soldier::takeDamage(amount);
    if (this->currentHP <= 0) {
        this->onDeath();
    }
}

RebelSoldier::RebelSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
{
    this->setEnemyType(ENEMY_REBEL);
    this->maxHealth = 9;
    this->currentHP = 9;
    this->health = 9;
    this->detectionRange = 600.f;
    this->attackRange = 500.f;
    this->attackCooldown = 1.2f;
    this->maxVelocity = 3.5f;
    this->baseMaxVelocity = 3.5f;
    this->scoreValue = 100;
    this->deathDuration = 1.5f;

    this->frameW = 70;
    this->frameH = 66;
    this->baseFrameW = 70;
    this->baseFrameH = 62;
    this->walkFrames = 9;
    this->shootFrames = 10;
    this->deathFrames = 8;

    Texture& walkTex = texMgr->getTexture("resources/Sprites/rebel-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(8);
    this->walkAnim.setFrameRect(0, 2, 2, 58, 62);
    this->walkAnim.setFrameRect(1, 61, 2, 63, 62);
    this->walkAnim.setFrameRect(2, 125, 3, 71, 53);
    this->walkAnim.setFrameRect(3, 197, 5, 70, 55);
    this->walkAnim.setFrameRect(4, 269, 3, 63, 62);
    this->walkAnim.setFrameRect(5, 333, 2, 58, 62);
    this->walkAnim.setFrameRect(6, 588, 5, 58, 54);
    this->walkAnim.setFrameRect(7, 653, 2, 58, 62);
    this->walkAnim.setFrameRect(8, 711, 2, 58, 62);
    this->walkAnim.setLoop(true);

    Texture& shootTex = texMgr->getTexture("resources/Sprites/rebel-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(6);
    this->shootAnim.setFrameRect(0, 2, 13, 74, 63);
    this->shootAnim.setFrameRect(1, 78, 13, 79, 63);
    this->shootAnim.setFrameRect(2, 158, 13, 80, 63);
    this->shootAnim.setFrameRect(3, 239, 10, 71, 66);
    this->shootAnim.setFrameRect(4, 311, 0, 69, 76);
    this->shootAnim.setFrameRect(5, 381, 0, 69, 76);
    this->shootAnim.setFrameRect(6, 452, 3, 72, 73);
    this->shootAnim.setFrameRect(7, 525, 13, 79, 63);
    this->shootAnim.setFrameRect(8, 605, 13, 77, 63);
    this->shootAnim.setFrameRect(9, 684, 13, 67, 63);
    this->shootAnim.setLoop(false);

    Texture& deathTex = texMgr->getTexture("resources/Sprites/rebel-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(this->deathFrames);
    this->deathAnim.setFrameDelay(10);
    this->deathAnim.setFrameRect(0, 5, 0, 62, 67);
    this->deathAnim.setFrameRect(1, 78, 0, 72, 67);
    this->deathAnim.setFrameRect(2, 158, 1, 74, 66);
    this->deathAnim.setFrameRect(3, 243, 10, 82, 58);
    this->deathAnim.setFrameRect(4, 335, 29, 80, 38);
    this->deathAnim.setFrameRect(5, 425, 33, 88, 34);
    this->deathAnim.setFrameRect(6, 523, 37, 90, 30);
    this->deathAnim.setFrameRect(7, 623, 37, 90, 30);
    this->deathAnim.setLoop(false);

    this->sprite.setTexture(walkTex);
    this->sprite.setTextureRect(IntRect(2, 2, 58, 62));
    this->sprite.setScale(2.25f, 2.25f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

RebelSoldier::~RebelSoldier() {}

void RebelSoldier::updateAI(PlayerSoldier* player, Level* lvl) {
    Enemy::updateAI(player, lvl);
}

void RebelSoldier::performAttack(PlayerSoldier* player) {
    Enemy::performAttack(player);
}

// ============================================================
// BazookaSoldier
// ============================================================

BazookaSoldier::BazookaSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
{
    this->setEnemyType(ENEMY_BAZOOKA);
    this->maxHealth = 12;
    this->currentHP = 12;
    this->health = 12;
    this->detectionRange = 700.f;
    this->attackRange = 600.f;
    this->attackCooldown = 3.0f;
    this->maxVelocity = 2.0f;
    this->baseMaxVelocity = 2.0f;
    this->scoreValue = 200;
    this->deathDuration = 1.5f;

    this->frameW = 200;
    this->frameH = 200;
    this->baseFrameW = 200;
    this->baseFrameH = 195;
    this->walkFrames = 5;
    this->shootFrames = 5;
    this->deathFrames = 8;

    Texture& walkTex = texMgr->getTexture("resources/Sprites/bazooka-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(8);
    this->walkAnim.setFrameRect(0, 5, 7, 205, 195);
    this->walkAnim.setFrameRect(1, 235, 2, 205, 200);
    this->walkAnim.setFrameRect(2, 470, 2, 200, 200);
    this->walkAnim.setFrameRect(3, 695, 7, 205, 195);
    this->walkAnim.setFrameRect(4, 925, 12, 200, 190);
    this->walkAnim.setLoop(true);

    Texture& shootTex = texMgr->getTexture("resources/Sprites/bazooka-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(6);
    this->shootAnim.setFrameRect(0, 0, 29, 193, 219);
    this->shootAnim.setFrameRect(1, 218, 0, 185, 248);
    this->shootAnim.setFrameRect(2, 428, 0, 175, 248);
    this->shootAnim.setFrameRect(3, 628, 0, 175, 248);
    this->shootAnim.setFrameRect(4, 828, 0, 175, 248);
    this->shootAnim.setLoop(false);

    Texture& deathTex = texMgr->getTexture("resources/Sprites/rebel-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(this->deathFrames);
    this->deathAnim.setFrameDelay(10);
    this->deathAnim.setFrameRect(0, 5, 0, 62, 67);
    this->deathAnim.setFrameRect(1, 78, 0, 72, 67);
    this->deathAnim.setFrameRect(2, 158, 1, 74, 66);
    this->deathAnim.setFrameRect(3, 243, 10, 82, 58);
    this->deathAnim.setFrameRect(4, 335, 29, 80, 38);
    this->deathAnim.setFrameRect(5, 425, 33, 88, 34);
    this->deathAnim.setFrameRect(6, 523, 37, 90, 30);
    this->deathAnim.setFrameRect(7, 623, 37, 90, 30);
    this->deathAnim.setLoop(false);

    this->sprite.setTexture(walkTex);
    this->sprite.setTextureRect(IntRect(5, 7, 205, 195));
    this->sprite.setScale(0.7f, 0.7f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

BazookaSoldier::~BazookaSoldier() {}

void BazookaSoldier::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.35f
    );

    // Calculate steep arc angle toward the player
    float dy = player->getPosition().y - this->position.y;
    float dx = fabsf(player->getPosition().x - this->position.x);
    float angle = 45.f;  // default steep arc
    if (dx > 0.f || dy != 0.f) {
        angle = atan2f(-dy, dx) * 180.f / 3.14159f;
        // Clamp to a steep arc: 30-70 degrees
        if (angle < 30.f) angle = 30.f;
        if (angle > 70.f) angle = 70.f;
    }

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    // Spawn explosive projectile with blast radius 3, 5 damage
    this->pm->spawnExplosive(origin, dir, angle, 5, 3, true);
}