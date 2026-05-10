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

// ============================================================
// RebelSoldier
// ============================================================

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

// ============================================================
// ShieldedSoldier
// ============================================================

ShieldedSoldier::ShieldedSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
    , hasShield(true)
    , shieldHP(3)
{
    this->setEnemyType(ENEMY_SHIELDED);
    this->maxHealth = 5;
    this->currentHP = 5;
    this->health = 5;
    this->detectionRange = 500.f;
    this->attackRange = 400.f;
    this->attackCooldown = 1.5f;
    this->maxVelocity = 2.5f;
    this->baseMaxVelocity = 2.5f;
    this->scoreValue = 75;
    this->deathDuration = 1.5f;

    this->frameW = 130;
    this->frameH = 169;
    this->baseFrameW = 128;
    this->baseFrameH = 165;
    this->walkFrames = 6;
    this->shootFrames = 10;
    this->deathFrames = 8;

    // Walk animation  shielded-walk.png (6 frames)
    Texture& walkTex = texMgr->getTexture("resources/Sprites/shielded-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(8);
    this->walkAnim.setFrameRect(0, 0, 0, 131, 169);
    this->walkAnim.setFrameRect(1, 131, 0, 126, 169);
    this->walkAnim.setFrameRect(2, 260, 0, 128, 169);
    this->walkAnim.setFrameRect(3, 393, 0, 127, 169);
    this->walkAnim.setFrameRect(4, 521, 0, 129, 169);
    this->walkAnim.setFrameRect(5, 655, 0, 131, 169);
    this->walkAnim.setLoop(true);

    // Shoot animation  shielded-shoot.png (10 frames)
    Texture& shootTex = texMgr->getTexture("resources/Sprites/shielded-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(6);
    this->shootAnim.setFrameRect(0, 9, 17, 128, 163);
    this->shootAnim.setFrameRect(1, 145, 17, 119, 163);
    this->shootAnim.setFrameRect(2, 272, 19, 133, 160);
    this->shootAnim.setFrameRect(3, 412, 17, 131, 163);
    this->shootAnim.setFrameRect(4, 551, 17, 161, 163);
    this->shootAnim.setFrameRect(5, 721, 17, 154, 163);
    this->shootAnim.setFrameRect(6, 883, 4, 148, 175);
    this->shootAnim.setFrameRect(7, 1039, 0, 144, 179);
    this->shootAnim.setFrameRect(8, 1190, 0, 149, 179);
    this->shootAnim.setFrameRect(9, 1347, 5, 155, 174);
    this->shootAnim.setLoop(false);

    // Death animation  shares rebel-death.png (8 frames)
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
    this->sprite.setTextureRect(IntRect(0, 0, 131, 169));
    this->sprite.setScale(0.85f, 0.85f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

ShieldedSoldier::~ShieldedSoldier() {}

void ShieldedSoldier::performAttack(PlayerSoldier* player) {
    // Shielded soldier shoots straight bullets like the rebel
    Enemy::performAttack(player);
}

void ShieldedSoldier::takeDamageFrom(int amount, int bulletDir) {
    if (this->dying) return;

    // Shield blocks frontal bullets. The enemy's facing direction determines
    // which side is "front". If the enemy faces right, bullets from the left
    // are frontal. If the enemy faces left, bullets from the right are frontal.
    if (this->hasShield && shieldHP > 0) {
        // bulletDir > 0 means bullet travels right (came from left = frontal if enemy faces right)
        // bulletDir < 0 means bullet travels left (came from right = frontal if enemy faces left)
        bool bulletFromFront = (this->faceRight && bulletDir > 0) ||
            (!this->faceRight && bulletDir < 0);

        if (bulletFromFront) {
            // Shield absorbs the hit
            this->shieldHP--;
            if (this->shieldHP <= 0) {
                this->hasShield = false;
            }
            return;  // Damage blocked by shield
        }
    }

    // No shield or hit from behind  take full damage
    this->takeDamage(amount);
}

// ============================================================
// GrenadeSoldier
// ============================================================

GrenadeSoldier::GrenadeSoldier(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
{
    this->setEnemyType(ENEMY_GRENADE);
    this->maxHealth = 2;
    this->currentHP = 2;
    this->health = 2;
    this->detectionRange = 550.f;
    this->attackRange = 450.f;
    this->attackCooldown = 2.5f;
    this->maxVelocity = 2.8f;
    this->baseMaxVelocity = 2.8f;
    this->scoreValue = 100;
    this->deathDuration = 1.5f;

    this->frameW = 70;
    this->frameH = 66;
    this->baseFrameW = 70;
    this->baseFrameH = 62;
    this->walkFrames = 9;
    this->shootFrames = 4;
    this->deathFrames = 8;

    // Walk animation  shares rebel-walk.png (9 frames)
    Texture& walkTex = texMgr->getTexture("resources/Sprites/rebel-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(8);
    this->walkAnim.setFrameRect(0, 0, 2, 58, 62);
    this->walkAnim.setFrameRect(1, 61, 2, 63, 62);
    this->walkAnim.setFrameRect(2, 125, 3, 71, 53);
    this->walkAnim.setFrameRect(3, 197, 5, 70, 55);
    this->walkAnim.setFrameRect(4, 269, 3, 63, 62);
    this->walkAnim.setFrameRect(5, 333, 2, 58, 62);
    this->walkAnim.setFrameRect(6, 588, 5, 58, 54);
    this->walkAnim.setFrameRect(7, 653, 2, 58, 62);
    this->walkAnim.setFrameRect(8, 711, 2, 58, 62);
    this->walkAnim.setLoop(true);

    // Shoot animation  grenade-shoot.png (4 frames)
    Texture& shootTex = texMgr->getTexture("resources/Sprites/grenade-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(10);
    this->shootAnim.setFrameRect(0, 4, 5, 150, 133);
    this->shootAnim.setFrameRect(1, 172, 0, 126, 138);
    this->shootAnim.setFrameRect(2, 315, 0, 116, 138);
    this->shootAnim.setFrameRect(3, 448, 15, 123, 123);
    this->shootAnim.setLoop(false);

    // Death animation  shares rebel-death.png (8 frames)
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
    this->sprite.setTextureRect(IntRect(0, 2, 58, 62));
    this->sprite.setScale(2.25f, 2.25f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

GrenadeSoldier::~GrenadeSoldier() {}

void GrenadeSoldier::draw(RenderWindow& window, float scrollX, float scrollY) {
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

    // grenade-shoot.png frames are ~2x larger than rebel-walk.png frames,
    // so use a smaller scale during the shoot animation to keep the visual
    // size consistent with the walk animation.
    float walkScale = 2.25f;
    float shootScale = 1.1f;
    float scale = (this->currentAnim == &this->shootAnim) ? shootScale : walkScale;

    if (this->dying) {
        scale = this->deathSpriteScale;
    }

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    float drawY = this->position.y - scrollY;
    if (this->dying) {
        drawY += 70.f;
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

void GrenadeSoldier::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.35f
    );

    // Lobs grenades in a parabolic arc toward the player
    float dy = player->getPosition().y - this->position.y;
    float dx = fabsf(player->getPosition().x - this->position.x);
    float angle = 45.f;  // default grenade arc
    if (dx > 0.f || dy != 0.f) {
        angle = atan2f(-dy, dx) * 180.f / 3.14159f;
        if (angle < 20.f) angle = 20.f;
        if (angle > 70.f) angle = 70.f;
    }

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    // Spawn explosive grenade: 3-block blast radius, 3 damage
    this->pm->spawnExplosive(origin, dir, angle, 3, 3, true);
}

// ============================================================
// Martian
// ============================================================

Martian::Martian(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
    , inPodPhase(true)
    , podHP(3)
{
    this->setEnemyType(ENEMY_MARTIAN);
    this->maxHealth = 3;
    this->currentHP = 3;
    this->health = 3;
    this->detectionRange = 600.f;
    this->attackRange = 500.f;
    this->attackCooldown = 1.8f;
    this->maxVelocity = 3.0f;
    this->baseMaxVelocity = 3.0f;
    this->scoreValue = 200;
    this->deathDuration = 2.0f;

    this->frameW = 200;
    this->frameH = 170;
    this->baseFrameW = 200;
    this->baseFrameH = 170;
    this->walkFrames = 16;
    this->shootFrames = 6;
    this->deathFrames = 8;

    // Walk animation  martian-walk.png (16 frames)
    Texture& walkTex = texMgr->getTexture("resources/Sprites/martian-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(5);
    this->walkAnim.setFrameRect(0, 0, 0, 196, 172);
    this->walkAnim.setFrameRect(1, 216, 0, 200, 172);
    this->walkAnim.setFrameRect(2, 436, 4, 200, 169);
    this->walkAnim.setFrameRect(3, 656, 4, 204, 169);
    this->walkAnim.setFrameRect(4, 880, 4, 208, 168);
    this->walkAnim.setFrameRect(5, 1108, 4, 204, 168);
    this->walkAnim.setFrameRect(6, 1332, 4, 204, 168);
    this->walkAnim.setFrameRect(7, 1556, 4, 208, 168);
    this->walkAnim.setFrameRect(8, 1784, 4, 208, 168);
    this->walkAnim.setFrameRect(9, 2012, 4, 208, 168);
    this->walkAnim.setFrameRect(10, 2240, 4, 208, 168);
    this->walkAnim.setFrameRect(11, 2468, 4, 200, 168);
    this->walkAnim.setFrameRect(12, 2692, 4, 200, 169);
    this->walkAnim.setFrameRect(13, 2904, 4, 192, 168);
    this->walkAnim.setFrameRect(14, 3116, 4, 192, 168);
    this->walkAnim.setFrameRect(15, 3328, 4, 192, 168);
    this->walkAnim.setLoop(true);

    // Shoot animation  martian-shoot.png (6 frames)
    Texture& shootTex = texMgr->getTexture("resources/Sprites/martian-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(8);
    this->shootAnim.setFrameRect(0, 0, 0, 276, 152);
    this->shootAnim.setFrameRect(1, 296, 4, 280, 148);
    this->shootAnim.setFrameRect(2, 596, 8, 300, 144);
    this->shootAnim.setFrameRect(3, 916, 12, 312, 140);
    this->shootAnim.setFrameRect(4, 1248, 8, 208, 144);
    this->shootAnim.setFrameRect(5, 1476, 4, 212, 148);
    this->shootAnim.setLoop(false);

    // Death animation  martian-death.png (8 frames)
    Texture& deathTex = texMgr->getTexture("resources/Sprites/martian-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(this->deathFrames);
    this->deathAnim.setFrameDelay(10);
    this->deathAnim.setFrameRect(0, 34, 80, 340, 368);
    this->deathAnim.setFrameRect(1, 435, 75, 346, 374);
    this->deathAnim.setFrameRect(2, 848, 80, 337, 375);
    this->deathAnim.setFrameRect(3, 1227, 96, 262, 352);
    this->deathAnim.setFrameRect(4, 47, 541, 329, 299);
    this->deathAnim.setFrameRect(5, 468, 553, 307, 291);
    this->deathAnim.setFrameRect(6, 831, 553, 307, 291);
    this->deathAnim.setFrameRect(7, 1224, 640, 266, 200);
    this->deathAnim.setLoop(false);

    this->sprite.setTexture(walkTex);
    this->sprite.setTextureRect(IntRect(0, 0, 196, 172));
    this->sprite.setScale(0.65f, 0.65f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

Martian::~Martian() {}

void Martian::updateAI(PlayerSoldier* player, Level* lvl) {
    // Martian uses base AI but transitions from pod to on-foot phase
    // when its pod HP is depleted
    if (this->inPodPhase && this->podHP <= 0) {
        this->inPodPhase = false;
        // Slightly faster and more aggressive on foot
        this->maxVelocity = 3.5f;
        this->baseMaxVelocity = 3.5f;
        this->attackCooldown = 1.0f;
    }
    Enemy::updateAI(player, lvl);
}

void Martian::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.4f
    );

    // Martian fires energy blasts  two rapid straight shots
    float angle = 0.f;
    float dy = player->getPosition().y - this->position.y;
    float dx = player->getPosition().x - this->position.x;
    if (dx != 0.f || dy != 0.f) {
        angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
        if (angle < -15.f) angle = -15.f;
        if (angle > 45.f) angle = 45.f;
    }

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    this->pm->spawnStraight(origin, dir, angle, 2, true);
}

void Martian::draw(RenderWindow& window, float scrollX, float scrollY) {
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

    // Martian walk/shoot frames are ~200px wide, but death frames are ~340px
    // wide and ~375px tall. Scale death down so it matches the alive size.
    float scale = 0.65f;  // walk/shoot scale (matches constructor)
    if (this->dying) {
        // death frames are much larger; 0.40f keeps them close to alive size
        scale = 0.40f;
    }
    else {
        // shoot frames can be wider (276-312px), use slightly smaller scale
        IntRect texRect = this->sprite.getTextureRect();
        if (texRect.width > 250) {
            scale = 0.70f;
        }
    }

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    float drawY = this->position.y - scrollY;
    if (this->dying) {
        drawY -= 20.f;  // move death sprite up so it doesn't sink into ground
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

// ============================================================
// Paratrooper
// ============================================================

Paratrooper::Paratrooper(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
    , paraState(0)        // start flying
    , landY(0.f)
    , fallSpeed(1.8f)     // slow descent: ~60 px/sec at 60fps
    , swayTimer(0.f)
    , startDescent(false) // wait for player to approach
    , triggerX(0.f)       // set via setTriggerX()
{
    this->setEnemyType(ENEMY_PARATROOPER);
    this->maxHealth = 5;
    this->currentHP = 5;
    this->health = 5;
    this->detectionRange = 600.f;
    this->attackRange = 500.f;
    this->attackCooldown = 1.2f;
    this->maxVelocity = 3.5f;
    this->baseMaxVelocity = 3.5f;
    this->scoreValue = 75;
    this->deathDuration = 1.5f;
    this->activated = true;  // always active  visible and swaying

    // Bounding box / frame size matches rebel soldier (used after landing)
    this->frameW = 70;
    this->frameH = 66;
    this->baseFrameW = 70;
    this->baseFrameH = 62;
    this->walkFrames = 9;
    this->shootFrames = 10;
    this->deathFrames = 8;

    // Fly animation  paratrooper.png (single frame)
    Texture& flyTex = texMgr->getTexture("resources/Sprites/paratrooper.png");
    this->flyAnim.setTexture(&flyTex);
    this->flyAnim.setFrameCount(1);
    this->flyAnim.setFrameDelay(1);
    this->flyAnim.setFrameRect(0, 181, 234, 681, 894);
    this->flyAnim.setLoop(true);

    // Walk animation  rebel-walk.png (9 frames, same as RebelSoldier)
    Texture& walkTex = texMgr->getTexture("resources/Sprites/rebel-walk.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(8);
    this->walkAnim.setFrameRect(0, 0, 2, 58, 62);
    this->walkAnim.setFrameRect(1, 61, 2, 63, 62);
    this->walkAnim.setFrameRect(2, 125, 3, 71, 53);
    this->walkAnim.setFrameRect(3, 197, 5, 70, 55);
    this->walkAnim.setFrameRect(4, 269, 3, 63, 62);
    this->walkAnim.setFrameRect(5, 333, 2, 58, 62);
    this->walkAnim.setFrameRect(6, 588, 5, 58, 54);
    this->walkAnim.setFrameRect(7, 653, 2, 58, 62);
    this->walkAnim.setFrameRect(8, 711, 2, 58, 62);
    this->walkAnim.setLoop(true);

    // Shoot animation  rebel-shoot.png (10 frames, same as RebelSoldier)
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

    // Death animation  rebel-death.png (8 frames, same as RebelSoldier)
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

    // Start with parachute sprite displayed
    this->sprite.setTexture(flyTex);
    this->sprite.setTextureRect(IntRect(181, 234, 681, 894));
    this->sprite.setScale(0.25f, 0.25f);
    this->switchAnim(&this->flyAnim);
    this->updateBoundingBox();
}

Paratrooper::~Paratrooper() {}

void Paratrooper::setLandY(float y) {
    this->landY = y;
}

void Paratrooper::setTriggerX(float x) {
    this->triggerX = x;
}

void Paratrooper::applyGravity() {
    if (this->paraState == 0) {
        if (this->startDescent) {
            // Slow, controlled descent with gentle sideways sway
            this->velocityY = this->fallSpeed;
            this->swayTimer += 0.03f;
            this->velocityX = sinf(this->swayTimer) * 0.5f;
        }
        else {
            // Waiting: hover in place with gentle sway, no descent
            this->velocityY = 0.f;
            this->swayTimer += 0.02f;
            this->velocityX = sinf(this->swayTimer) * 0.3f;
        }
    }
    else {
        // Normal gravity when grounded
        Soldier::applyGravity();
    }
}

void Paratrooper::handleCollision(Level* lvl) {
    if (this->paraState == 0) {
        // Skip level collision while parachuting; landing is handled
        // by the landY check in updateAI()
        this->onGround = false;
        return;
    }
    Enemy::handleCollision(lvl);
}

void Paratrooper::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) return;

    if (this->paraState == 0) {
        // Check if player has walked close enough to trigger descent
        if (!this->startDescent && player != nullptr) {
            float dx = fabsf(player->getPosition().x - this->position.x);
            float dy = fabsf(player->getPosition().y - this->landY);
            // Player must be within 800px horizontally AND within 500px
            // vertically of the landing spot  means they're climbing
            // the mountain toward the paratrooper
            if (dx < 200.f && dy < 500.f) {
                this->startDescent = true;
            }
        }

        // If descent has started, check if reached landing position
        if (this->startDescent && this->landY > 0.f && this->position.y >= this->landY) {
            this->paraState = 1;   // transition to grounded
            this->position.y = this->landY;
            this->velocityY = 0.f;
            this->velocityX = 0.f;
            this->onGround = true;
            this->faceRight = false;
            this->switchAnim(&this->walkAnim);
        }
        // While flying: no attack, no chase  just descend (or hover)
        return;
    }

    // Grounded: use normal rebel AI (patrol / chase / attack)
    Enemy::updateAI(player, lvl);
}

void Paratrooper::performAttack(PlayerSoldier* player) {
    // Same as rebel soldier  straight bullets
    Enemy::performAttack(player);
}

void Paratrooper::onDeath() {
    if (this->dying) return;
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    // If still flying, switch to grounded so normal gravity + collision
    // apply and the body falls naturally
    this->paraState = 1;
    this->switchAnim(&this->deathAnim);
}

void Paratrooper::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
            return;
        }
    }

    if (this->paraState == 0) {
        // === FLYING: draw parachute sprite ===
        this->flyAnim.update();
        this->flyAnim.applyToSprite(this->sprite);

        float flyScale = 0.25f;
        if (this->faceRight) {
            this->sprite.setScale(-flyScale, flyScale);
        }
        else {
            this->sprite.setScale(flyScale, flyScale);
        }

        // The parachute frame (681x894) has the soldier at the bottom.
        // Offset upward so the soldier body aligns with position.y
        float spriteH = 894.f * flyScale;
        float rebelVisH = 62.f * 2.25f;
        float yOff = -(spriteH - rebelVisH);

        float drawX = this->position.x - scrollX;
        float drawY = this->position.y - scrollY + yOff;

        this->sprite.setPosition(drawX, drawY);
        window.draw(this->sprite);
    }
    else {
        // === GROUNDED: draw like a rebel soldier ===
        if (this->currentAnim != nullptr) {
            this->currentAnim->update();
            this->currentAnim->applyToSprite(this->sprite);
        }

        float scale = 2.25f;  // walk scale (default)
        if (!this->dying) {
            IntRect texRect = this->sprite.getTextureRect();
            if (texRect.width > 100) {  // shoot frames are larger
                scale = 1.1f;
            }
        }
        else {
            scale = this->deathSpriteScale;  // 2.0f
        }

        if (this->faceRight) {
            this->sprite.setScale(-scale, scale);
        }
        else {
            this->sprite.setScale(scale, scale);
        }

        float drawY = this->position.y - scrollY;
        if (this->dying) {
            drawY += 70.f;
        }

        this->sprite.setPosition(this->position.x - scrollX, drawY);
        window.draw(this->sprite);
    }
}

// ============================================================
// Boss — base class for all boss enemies
// ============================================================

Boss::Boss(TextureManager* texMgr, AudioManager* audMgr)
    : Enemy(texMgr, audMgr)
    , bossPhase(0)
    , phase2Threshold(0.5f)
    , bossName("UNKNOWN BOSS")
    , entranceDone(false)
    , entranceTimer(0.f)
    , specialCooldown(5.0f)
    , chargeCooldown(4.0f)
    , chargeSpeed(8.0f)
    , isCharging(false)
    , chargeDuration(1.0f)
    , chargeElapsed(0.f)
{
    this->activated = true;  // bosses are always active
    this->detectionRange = 2000.f;
    this->attackRange = 800.f;
    this->attackCooldown = 2.0f;
    this->maxVelocity = 2.0f;
    this->baseMaxVelocity = 2.0f;
    this->deathDuration = 3.0f;  // bosses have longer death animations
    this->deathSpriteScale = 2.5f;

    this->idleAnim.setLoop(true);
    this->chargeAnim.setLoop(true);
    this->specialAnim.setLoop(false);
}

Boss::~Boss() {}

const char* Boss::getBossName() const {
    return this->bossName;
}

float Boss::getHealthFraction() const {
    if (this->maxHealth <= 0) return 0.f;
    float frac = (float)(this->currentHP) / (float)(this->maxHealth);
    if (frac < 0.f) frac = 0.f;
    if (frac > 1.f) frac = 1.f;
    return frac;
}

bool Boss::isEntranceDone() const {
    return this->entranceDone;
}

void Boss::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) return;
    if (player == nullptr) return;

    // Entrance period: boss stands still for 2 seconds
    if (!this->entranceDone) {
        this->entranceTimer += 1.f / 60.f;
        if (this->entranceTimer >= 2.0f) {
            this->entranceDone = true;
        }
        this->velocityX = 0.f;
        this->switchAnim(&this->idleAnim);
        return;
    }

    // Check phase transition
    if (this->bossPhase == 0 && this->getHealthFraction() <= this->phase2Threshold) {
        this->bossPhase = 1;
        this->maxVelocity = this->baseMaxVelocity * 1.5f;
        this->attackCooldown *= 0.6f;
        this->specialCooldown *= 0.7f;
        this->chargeSpeed *= 1.3f;
    }

    // Handle charging
    if (this->isCharging) {
        this->chargeElapsed += 1.f / 60.f;
        if (this->chargeElapsed >= this->chargeDuration) {
            this->isCharging = false;
            this->velocityX = 0.f;
        }
        else {
            float dir = this->faceRight ? 1.f : -1.f;
            this->velocityX = dir * this->chargeSpeed;
        }
        return;
    }

    float px = player->getPosition().x;
    this->faceRight = (px > this->position.x);

    // Decide action: attack, charge, special, or walk
    float dist = this->distanceTo(player);

    if (dist <= this->attackRange) {
        // In attack range: decide between normal attack, charge, or special
        bool doSpecial = this->specialTimer.getElapsedTime().asSeconds() >= this->specialCooldown;
        bool doCharge = !doSpecial && this->chargeTimer.getElapsedTime().asSeconds() >= this->chargeCooldown;

        if (doSpecial) {
            this->aiState = AI_BOSS_SPECIAL;
            this->velocityX = 0.f;
            this->performAttack(player);
            this->specialTimer.restart();
            this->switchAnim(&this->specialAnim);
        }
        else if (doCharge) {
            this->aiState = AI_BOSS_CHARGE;
            this->isCharging = true;
            this->chargeElapsed = 0.f;
            this->chargeTimer.restart();
            this->switchAnim(&this->chargeAnim);
        }
        else if (this->attackTimer.getElapsedTime().asSeconds() >= this->attackCooldown) {
            this->aiState = AI_BOSS_ATTACK;
            this->velocityX = 0.f;
            this->performAttack(player);
            this->attackTimer.restart();
            this->switchAnim(&this->shootAnim);
        }
        else {
            this->switchAnim(&this->idleAnim);
        }
    }
    else {
        // Walk toward player
        this->aiState = AI_BOSS_WALK;
        this->switchAnim(&this->walkAnim);

        if (px > this->position.x + 20.f) {
            this->velocityX += 0.5f;
            if (this->velocityX > this->maxVelocity)
                this->velocityX = this->maxVelocity;
        }
        else if (px < this->position.x - 20.f) {
            this->velocityX -= 0.5f;
            if (this->velocityX < -this->maxVelocity)
                this->velocityX = -this->maxVelocity;
        }
        else {
            this->decelerate();
        }
    }
}

void Boss::onDeath() {
    if (this->dying) return;
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->isCharging = false;
    this->switchAnim(&this->deathAnim);
}

void Boss::draw(RenderWindow& window, float scrollX, float scrollY) {
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
        drawY += 40.f;
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

// ============================================================
// Ironokava — first boss
// ============================================================

Ironokava::Ironokava(TextureManager* texMgr, AudioManager* audMgr)
    : Boss(texMgr, audMgr)
{
    this->setEnemyType(ENEMY_BOSS_IRONOKAVA);
    this->bossName = "IRONOKAVA";
    this->maxHealth = 200;
    this->currentHP = 200;
    this->health = 200;
    this->scoreValue = 5000;
    this->detectionRange = 2000.f;
    this->attackRange = 700.f;
    this->attackCooldown = 2.0f;
    this->maxVelocity = 2.5f;
    this->baseMaxVelocity = 2.5f;
    this->deathDuration = 3.5f;

    // Ironokava sprite dimensions (idle/walk frames)
    this->frameW = 176;
    this->frameH = 120;
    this->baseFrameW = 176;
    this->baseFrameH = 120;
    this->walkFrames = 2;    // ironokava.png has 2 frames
    this->shootFrames = 2;   // reuse walk frames for attack stance
    this->deathFrames = 4;   // ironokava-death.png has 4 frames

    this->specialCooldown = 5.0f;
    this->chargeCooldown = 4.0f;
    this->chargeSpeed = 8.0f;
    this->chargeDuration = 1.2f;

    // Walk/idle animation — ironokava.png (2 frames)
    Texture& walkTex = texMgr->getTexture("resources/Sprites/ironokava.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(15);
    this->walkAnim.setFrameRect(0, 0, 0, 176, 120);
    this->walkAnim.setFrameRect(1, 180, 1, 175, 120);
    this->walkAnim.setLoop(true);

    // Shoot animation — reuse ironokava.png (same 2 frames, slower = attack pose)
    this->shootAnim.setTexture(&walkTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(8);
    this->shootAnim.setFrameRect(0, 0, 0, 176, 120);
    this->shootAnim.setFrameRect(1, 180, 1, 175, 120);
    this->shootAnim.setLoop(false);

    // Death animation — ironokava-death.png (4 frames)
    Texture& deathTex = texMgr->getTexture("resources/Sprites/ironokava-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(this->deathFrames);
    this->deathAnim.setFrameDelay(12);
    this->deathAnim.setFrameRect(0, 4, 95, 228, 150);
    this->deathAnim.setFrameRect(1, 232, 34, 222, 196);
    this->deathAnim.setFrameRect(2, 442, 40, 220, 199);
    this->deathAnim.setFrameRect(3, 672, 90, 199, 147);
    this->deathAnim.setLoop(false);

    // Idle animation — reuse walk frames at slower speed
    this->idleAnim.setTexture(&walkTex);
    this->idleAnim.setFrameCount(this->walkFrames);
    this->idleAnim.setFrameDelay(25);
    this->idleAnim.setFrameRect(0, 0, 0, 176, 120);
    this->idleAnim.setFrameRect(1, 180, 1, 175, 120);
    this->idleAnim.setLoop(true);

    // Charge animation — reuse walk frames at faster speed
    this->chargeAnim.setTexture(&walkTex);
    this->chargeAnim.setFrameCount(this->walkFrames);
    this->chargeAnim.setFrameDelay(4);
    this->chargeAnim.setFrameRect(0, 0, 0, 176, 120);
    this->chargeAnim.setFrameRect(1, 180, 1, 175, 120);
    this->chargeAnim.setLoop(true);

    // Special attack animation — reuse walk
    this->specialAnim.setTexture(&walkTex);
    this->specialAnim.setFrameCount(this->walkFrames);
    this->specialAnim.setFrameDelay(6);
    this->specialAnim.setFrameRect(0, 0, 0, 176, 120);
    this->specialAnim.setFrameRect(1, 180, 1, 175, 120);
    this->specialAnim.setLoop(false);

    this->sprite.setTexture(walkTex);
    this->sprite.setTextureRect(IntRect(0, 0, 176, 120));
    this->sprite.setScale(3.0f, 3.0f);
    this->switchAnim(&this->idleAnim);
    this->updateBoundingBox();
}

Ironokava::~Ironokava() {}

void Ironokava::updateAI(PlayerSoldier* player, Level* lvl) {
    Boss::updateAI(player, lvl);
}

void Ironokava::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.4f
    );

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    if (this->aiState == AI_BOSS_SPECIAL) {
        // Special attack: spread of 3 explosive projectiles
        float baseAngle = 0.f;
        float dy = player->getPosition().y - this->position.y;
        float dx = player->getPosition().x - this->position.x;
        if (dx != 0.f || dy != 0.f) {
            baseAngle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
            if (baseAngle < -20.f) baseAngle = -20.f;
            if (baseAngle > 45.f) baseAngle = 45.f;
        }

        // Three shots in a fan pattern
        this->pm->spawnExplosive(origin, dir, baseAngle - 15.f, 4, 3, true);
        this->pm->spawnExplosive(origin, dir, baseAngle, 4, 3, true);
        this->pm->spawnExplosive(origin, dir, baseAngle + 15.f, 4, 3, true);
    }
    else {
        // Normal attack: single heavy explosive projectile aimed at player
        float angle = 0.f;
        float dy = player->getPosition().y - this->position.y;
        float dx = player->getPosition().x - this->position.x;
        if (dx != 0.f || dy != 0.f) {
            angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
            if (angle < -15.f) angle = -15.f;
            if (angle > 45.f) angle = 45.f;
        }
        this->pm->spawnExplosive(origin, dir, angle, 3, 2, true);

        // Phase 2: fire an additional straight shot
        if (this->bossPhase >= 1) {
            this->pm->spawnStraight(origin, dir, angle, 3, true);
        }
    }
}

void Ironokava::draw(RenderWindow& window, float scrollX, float scrollY) {
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

    // Ironokava walk/idle frames are ~176x120, death frames vary 199-228 wide, 147-199 tall
    float scale = 3.0f;
    if (this->dying) {
        // Death frames are larger, scale down to match alive size
        IntRect texRect = this->sprite.getTextureRect();
        if (texRect.width > 180) {
            scale = 2.3f;
        }
    }
    else if (this->isCharging) {
        scale = 3.2f;  // slightly bigger when charging for visual emphasis
    }

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    float drawY = this->position.y - scrollY;
    if (this->dying) {
        drawY += 30.f;
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}