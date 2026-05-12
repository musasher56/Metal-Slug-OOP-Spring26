#include "Enemy.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(TextureManager* texMgr, AudioManager* audMgr) :Soldier(texMgr, audMgr), aiState(AI_IDLE), detectionRange(400.f)
, attackRange(300.f), attackCooldown(1.5f), pm(nullptr), activated(false), patrolCenter(0.f), patrolRadius(100.f), enemyType(ENEMY_REBEL)
, currentAnim(nullptr), frameW(78), frameH(66), baseFrameW(78), baseFrameH(66), walkFrames(10), shootFrames(8), deathFrames(6)
, dying(false), deathDuration(1.0f), deathSpriteScale(2.0f), faceRight(false)
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
    if (player == nullptr) {
        return 9999.f;
    }
    float dx = player->getPosition().x - this->position.x;
    float dy = player->getPosition().y - this->position.y;
    return sqrtf(dx * dx + dy * dy);
}

bool Enemy::playerInRange(PlayerSoldier* player, float range) const {
    return this->distanceTo(player) <= range;
}

void Enemy::switchAnim(Animation* newAnim) {
    if (this->currentAnim == newAnim) {
        return;
    }
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
    if (player == nullptr) {
        return;
    }
    if (this->dying) {
        return;
    }

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
            if (this->velocityX > this->maxVelocity) {
                this->velocityX = this->maxVelocity;
            }
        }
        else if (px < this->position.x - 10.f) {
            this->velocityX -= 0.3f;
            if (this->velocityX < -this->maxVelocity) {
                this->velocityX = -this->maxVelocity;
            }
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
    if (this->pm == nullptr || player == nullptr) {
        return;
    }
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
        if (angle < 0.f) {
            angle = 0.f;
        }
        if (angle > 45.f) {
            angle = 45.f;
        }
    }
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    this->pm->spawnStraight(origin, dir, angle, 1, true);
}

void Enemy::onDeath() {
    if (this->dying) {
        return;
    }
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
    if (!this->status) {
        return;
    }
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
    if (lvl == nullptr) {
        return;
    }
    if (this->dying) {
        return;
    }
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float enemyLeft = this->position.x;
    float enemyRight = this->position.x + (float)(this->baseFrameW) * scaleX;
    float enemyTop = this->position.y;
    float enemyBottom = this->position.y + (float)(this->baseFrameH) * scaleY;
    int cellSize = lvl->getCellSize();
    int worldOffX = lvl->getWorldOffX();
    this->onGround = false;
    int startCol = static_cast<int>(enemyLeft) / cellSize - worldOffX - 1;
    int endCol = static_cast<int>(enemyRight) / cellSize - worldOffX + 1;
    int startRow = static_cast<int>(enemyTop) / cellSize - 1;
    int endRow = static_cast<int>(enemyBottom) / cellSize + 1;

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) {
                continue;
            }

            float blockLeft = static_cast<float>((worldOffX + col) * cellSize);
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
        int probeStartCol = static_cast<int>(enemyLeft + 2) / cellSize - worldOffX;
        int probeEndCol = static_cast<int>(enemyRight - 2) / cellSize - worldOffX;
        for (int col = probeStartCol; col <= probeEndCol; ++col) {
            if (lvl->isSolid(probeRow, col)) {
                this->onGround = true;
                break;
            }
        }
    }
}

int Enemy::getAIState() const {
    return this->aiState;
}
int Enemy::getEnemyType() const {
    return this->enemyType;
}

void Enemy::takeDamage(int amount) {
    if (this->dying) {
        return;
    }
    Soldier::takeDamage(amount);
    if (this->currentHP <= 0) {
        this->onDeath();
    }
}

RebelSoldier::RebelSoldier(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr)
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

BazookaSoldier::BazookaSoldier(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr)
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
    if (this->pm == nullptr || player == nullptr) {
        return;
    }
    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.35f
    );
    float dy = player->getPosition().y - this->position.y;
    float dx = fabsf(player->getPosition().x - this->position.x);
    float angle = 45.f;
    if (dx > 0.f || dy != 0.f) {
        angle = atan2f(-dy, dx) * 180.f / 3.14159f;
        if (angle < 30.f) {
            angle = 30.f;
        }
        if (angle > 70.f) {
            angle = 70.f;
        }
    }
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    this->pm->spawnExplosive(origin, dir, angle, 5, 3, true);
}

ShieldedSoldier::ShieldedSoldier(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr), hasShield(true), shieldHP(3)
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
    Enemy::performAttack(player);
}

void ShieldedSoldier::takeDamageFrom(int amount, int bulletDir) {
    if (this->dying) {
        return;
    }
    if (this->hasShield && shieldHP > 0) {
        bool bulletFromFront = (this->faceRight && bulletDir > 0) ||
            (!this->faceRight && bulletDir < 0);

        if (bulletFromFront) {
            this->shieldHP--;
            if (this->shieldHP <= 0) {
                this->hasShield = false;
            }
            return;
        }
    }
    this->takeDamage(amount);
}

GrenadeSoldier::GrenadeSoldier(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr)
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

    Texture& shootTex = texMgr->getTexture("resources/Sprites/grenade-shoot.png");
    this->shootAnim.setTexture(&shootTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(10);
    this->shootAnim.setFrameRect(0, 4, 5, 150, 133);
    this->shootAnim.setFrameRect(1, 172, 0, 126, 138);
    this->shootAnim.setFrameRect(2, 315, 0, 116, 138);
    this->shootAnim.setFrameRect(3, 448, 15, 123, 123);
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
    this->sprite.setTextureRect(IntRect(0, 2, 58, 62));
    this->sprite.setScale(2.25f, 2.25f);
    this->switchAnim(&this->walkAnim);
    this->updateBoundingBox();
}

GrenadeSoldier::~GrenadeSoldier() {}

void GrenadeSoldier::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

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
    if (this->pm == nullptr || player == nullptr) {
        return;
    }

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.35f
    );

    float dy = player->getPosition().y - this->position.y;
    float dx = fabsf(player->getPosition().x - this->position.x);
    float angle = 45.f;
    if (dx > 0.f || dy != 0.f) {
        angle = atan2f(-dy, dx) * 180.f / 3.14159f;
        if (angle < 20.f) {
            angle = 20.f;
        }
        if (angle > 70.f) {
            angle = 70.f;
        }
    }

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    this->pm->spawnExplosive(origin, dir, angle, 3, 3, true);
}

Martian::Martian(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr), inPodPhase(true), podHP(3)
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
    if (this->inPodPhase && this->podHP <= 0) {
        this->inPodPhase = false;
        this->maxVelocity = 3.5f;
        this->baseMaxVelocity = 3.5f;
        this->attackCooldown = 1.0f;
    }
    Enemy::updateAI(player, lvl);
}

void Martian::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) {
        return;
    }
    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.4f
    );
    float angle = 0.f;
    float dy = player->getPosition().y - this->position.y;
    float dx = player->getPosition().x - this->position.x;
    if (dx != 0.f || dy != 0.f) {
        angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
        if (angle < -15.f) {
            angle = -15.f;
        }
        if (angle > 45.f) {
            angle = 45.f;
        }
    }
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;
    this->pm->spawnStraight(origin, dir, angle, 2, true);
}

void Martian::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }
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
    float scale = 0.65f;
    if (this->dying) {
        scale = 0.40f;
    }
    else {
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
        drawY -= 20.f;
    }
    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

Paratrooper::Paratrooper(TextureManager* texMgr, AudioManager* audMgr) :Enemy(texMgr, audMgr)
, paraState(0), landY(0.f), fallSpeed(1.8f), swayTimer(0.f), startDescent(false), triggerX(0.f)
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
    this->activated = true;
    this->frameW = 70;
    this->frameH = 66;
    this->baseFrameW = 70;
    this->baseFrameH = 62;
    this->walkFrames = 9;
    this->shootFrames = 10;
    this->deathFrames = 8;

    Texture& flyTex = texMgr->getTexture("resources/Sprites/paratrooper.png");
    this->flyAnim.setTexture(&flyTex);
    this->flyAnim.setFrameCount(1);
    this->flyAnim.setFrameDelay(1);
    this->flyAnim.setFrameRect(0, 181, 234, 681, 894);
    this->flyAnim.setLoop(true);
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
            this->velocityY = this->fallSpeed;
            this->swayTimer += 0.03f;
            this->velocityX = sinf(this->swayTimer) * 0.5f;
        }
        else {
            this->velocityY = 0.f;
            this->swayTimer += 0.02f;
            this->velocityX = sinf(this->swayTimer) * 0.3f;
        }
    }
    else {
        Soldier::applyGravity();
    }
}

void Paratrooper::handleCollision(Level* lvl) {
    if (this->paraState == 0) {
        this->onGround = false;
        return;
    }
    Enemy::handleCollision(lvl);
}

void Paratrooper::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) {
        return;
    }
    if (this->paraState == 0) {
        if (!this->startDescent && player != nullptr) {
            float dx = fabsf(player->getPosition().x - this->position.x);
            float dy = fabsf(player->getPosition().y - this->landY);
            if (dx < 200.f && dy < 500.f) {
                this->startDescent = true;
            }
        }
        if (this->startDescent && this->landY > 0.f && this->position.y >= this->landY) {
            this->paraState = 1;
            this->position.y = this->landY;
            this->velocityY = 0.f;
            this->velocityX = 0.f;
            this->onGround = true;
            this->faceRight = false;
            this->switchAnim(&this->walkAnim);
        }
        return;
    }
    Enemy::updateAI(player, lvl);
}

void Paratrooper::performAttack(PlayerSoldier* player) {
    Enemy::performAttack(player);
}

void Paratrooper::onDeath() {
    if (this->dying) {
        return;
    }
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->paraState = 1;
    this->switchAnim(&this->deathAnim);
}

void Paratrooper::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
            return;
        }
    }
    if (this->paraState == 0) {
        this->flyAnim.update();
        this->flyAnim.applyToSprite(this->sprite);

        float flyScale = 0.25f;
        if (this->faceRight) {
            this->sprite.setScale(-flyScale, flyScale);
        }
        else {
            this->sprite.setScale(flyScale, flyScale);
        }
        float spriteH = 894.f * flyScale;
        float rebelVisH = 62.f * 2.25f;
        float yOff = -(spriteH - rebelVisH);
        float drawX = this->position.x - scrollX;
        float drawY = this->position.y - scrollY + yOff;
        this->sprite.setPosition(drawX, drawY);
        window.draw(this->sprite);
    }
    else {
        if (this->currentAnim != nullptr) {
            this->currentAnim->update();
            this->currentAnim->applyToSprite(this->sprite);
        }

        float scale = 2.25f;
        if (!this->dying) {
            IntRect texRect = this->sprite.getTextureRect();
            if (texRect.width > 100) {
                scale = 1.1f;
            }
        }
        else {
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
}

Boss::Boss(TextureManager* texMgr, AudioManager* audMgr) : Enemy(texMgr, audMgr), bossPhase(0), phase2Threshold(0.5f)
, bossName("UNKNOWN BOSS"), entranceDone(false), entranceTimer(0.f), specialCooldown(5.0f), chargeCooldown(4.0f),
chargeSpeed(8.0f), isCharging(false), chargeDuration(1.0f), chargeElapsed(0.f)
{
    this->activated = true;
    this->detectionRange = 2000.f;
    this->attackRange = 800.f;
    this->attackCooldown = 2.0f;
    this->maxVelocity = 2.0f;
    this->baseMaxVelocity = 2.0f;
    this->deathDuration = 3.0f;
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
    if (this->maxHealth <= 0) {
        return 0.f;
    }
    float frac = (float)(this->currentHP) / (float)(this->maxHealth);
    if (frac < 0.f) {
        frac = 0.f;
    }
    if (frac > 1.f) {
        frac = 1.f;
    }
    return frac;
}

bool Boss::isEntranceDone() const {
    return this->entranceDone;
}

void Boss::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }
        return;
    }
    if (player == nullptr) {
        return;
    }

    if (!this->entranceDone) {
        this->entranceTimer += 1.f / 60.f;
        if (this->entranceTimer >= 2.0f) {
            this->entranceDone = true;
        }
        this->velocityX = 0.f;
        this->switchAnim(&this->idleAnim);
        return;
    }

    if (this->bossPhase == 0 && this->getHealthFraction() <= this->phase2Threshold) {
        this->bossPhase = 1;
        this->maxVelocity = this->baseMaxVelocity * 1.5f;
        this->attackCooldown *= 0.6f;
        this->specialCooldown *= 0.7f;
        this->chargeSpeed *= 1.3f;
    }

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

    float dist = this->distanceTo(player);

    if (dist <= this->attackRange) {
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
        this->aiState = AI_BOSS_WALK;
        this->switchAnim(&this->walkAnim);

        if (px > this->position.x + 20.f) {
            this->velocityX += 0.5f;
            if (this->velocityX > this->maxVelocity) {
                this->velocityX = this->maxVelocity;
            }
        }
        else if (px < this->position.x - 20.f) {
            this->velocityX -= 0.5f;
            if (this->velocityX < -this->maxVelocity) {
                this->velocityX = -this->maxVelocity;
            }
        }
        else {
            this->decelerate();
        }
    }
}

void Boss::onDeath() {
    if (this->dying) {
        return;
    }
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->isCharging = false;
    this->switchAnim(&this->deathAnim);
}

void Boss::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

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

Hairbuster::Hairbuster(TextureManager* texMgr, AudioManager* audMgr)
    : Boss(texMgr, audMgr)
    , flyCenterX(1500.f), flyCenterY(400.f), flyRadiusX(400.f), flyRadiusY(180.f), flyAngle(0.f), flySpeed(0.02f),
    diveTargetX(0.f), diveTargetY(0.f), isDiving(false), diveSpeed(6.0f), diveTimer(0.f), diveDuration(1.0f), bombCooldown(2.5f)
{
    this->setEnemyType(ENEMY_BOSS_HAIRBUSTER);
    this->bossName = "HAIRBUSTER";
    this->maxHealth = 120;
    this->currentHP = 120;
    this->health = 120;
    this->scoreValue = 8000;
    this->detectionRange = 3000.f;
    this->attackRange = 1200.f;
    this->attackCooldown = 1.5f;
    this->maxVelocity = 4.0f;
    this->baseMaxVelocity = 4.0f;
    this->deathDuration = 3.0f;
    this->deathSpriteScale = 2.0f;
    this->onGround = false;
    this->frameW = 179;
    this->frameH = 121;
    this->baseFrameW = 179;
    this->baseFrameH = 108;
    this->walkFrames = 12;
    this->shootFrames = 12;
    this->deathFrames = 3;
    this->specialCooldown = 4.0f;
    this->chargeCooldown = 6.0f;
    this->chargeSpeed = 7.0f;
    this->chargeDuration = 1.5f;
    this->phase2Threshold = 0.4f;

    Texture& flyTex = texMgr->getTexture("resources/Sprites/hairbuster.png");
    this->walkAnim.setTexture(&flyTex);
    this->walkAnim.setFrameCount(12);
    this->walkAnim.setFrameDelay(6);
    this->walkAnim.setFrameRect(0, 3, 2, 179, 121);
    this->walkAnim.setFrameRect(1, 186, 16, 179, 108);
    this->walkAnim.setFrameRect(2, 369, 18, 179, 105);
    this->walkAnim.setFrameRect(3, 552, 18, 179, 104);
    this->walkAnim.setFrameRect(4, 735, 15, 179, 108);
    this->walkAnim.setFrameRect(5, 918, 19, 179, 105);
    this->walkAnim.setFrameRect(6, 1101, 19, 179, 105);
    this->walkAnim.setFrameRect(7, 1284, 14, 179, 108);
    this->walkAnim.setFrameRect(8, 1467, 18, 179, 105);
    this->walkAnim.setFrameRect(9, 1650, 20, 179, 104);
    this->walkAnim.setFrameRect(10, 1833, 15, 179, 108);
    this->walkAnim.setFrameRect(11, 2016, 17, 179, 105);
    this->walkAnim.setLoop(true);

    this->shootAnim.setTexture(&flyTex);
    this->shootAnim.setFrameCount(12);
    this->shootAnim.setFrameDelay(4);
    this->shootAnim.setFrameRect(0, 3, 2, 179, 121);
    this->shootAnim.setFrameRect(1, 186, 16, 179, 108);
    this->shootAnim.setFrameRect(2, 369, 18, 179, 105);
    this->shootAnim.setFrameRect(3, 552, 18, 179, 104);
    this->shootAnim.setFrameRect(4, 735, 15, 179, 108);
    this->shootAnim.setFrameRect(5, 918, 19, 179, 105);
    this->shootAnim.setFrameRect(6, 1101, 19, 179, 105);
    this->shootAnim.setFrameRect(7, 1284, 14, 179, 108);
    this->shootAnim.setFrameRect(8, 1467, 18, 179, 105);
    this->shootAnim.setFrameRect(9, 1650, 20, 179, 104);
    this->shootAnim.setFrameRect(10, 1833, 15, 179, 108);
    this->shootAnim.setFrameRect(11, 2016, 17, 179, 105);
    this->shootAnim.setLoop(false);

    Texture& deathTex = texMgr->getTexture("resources/Sprites/hairbuster-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(3);
    this->deathAnim.setFrameDelay(15);
    this->deathAnim.setFrameRect(0, 8, 286, 470, 418);
    this->deathAnim.setFrameRect(1, 487, 280, 496, 428);
    this->deathAnim.setFrameRect(2, 993, 241, 526, 479);
    this->deathAnim.setLoop(false);

    this->idleAnim.setTexture(&flyTex);
    this->idleAnim.setFrameCount(12);
    this->idleAnim.setFrameDelay(10);
    this->idleAnim.setFrameRect(0, 3, 2, 179, 121);
    this->idleAnim.setFrameRect(1, 186, 16, 179, 108);
    this->idleAnim.setFrameRect(2, 369, 18, 179, 105);
    this->idleAnim.setFrameRect(3, 552, 18, 179, 104);
    this->idleAnim.setFrameRect(4, 735, 15, 179, 108);
    this->idleAnim.setFrameRect(5, 918, 19, 179, 105);
    this->idleAnim.setFrameRect(6, 1101, 19, 179, 105);
    this->idleAnim.setFrameRect(7, 1284, 14, 179, 108);
    this->idleAnim.setFrameRect(8, 1467, 18, 179, 105);
    this->idleAnim.setFrameRect(9, 1650, 20, 179, 104);
    this->idleAnim.setFrameRect(10, 1833, 15, 179, 108);
    this->idleAnim.setFrameRect(11, 2016, 17, 179, 105);
    this->idleAnim.setLoop(true);

    this->chargeAnim.setTexture(&flyTex);
    this->chargeAnim.setFrameCount(12);
    this->chargeAnim.setFrameDelay(3);
    this->chargeAnim.setFrameRect(0, 3, 2, 179, 121);
    this->chargeAnim.setFrameRect(1, 186, 16, 179, 108);
    this->chargeAnim.setFrameRect(2, 369, 18, 179, 105);
    this->chargeAnim.setFrameRect(3, 552, 18, 179, 104);
    this->chargeAnim.setFrameRect(4, 735, 15, 179, 108);
    this->chargeAnim.setFrameRect(5, 918, 19, 179, 105);
    this->chargeAnim.setFrameRect(6, 1101, 19, 179, 105);
    this->chargeAnim.setFrameRect(7, 1284, 14, 179, 108);
    this->chargeAnim.setFrameRect(8, 1467, 18, 179, 105);
    this->chargeAnim.setFrameRect(9, 1650, 20, 179, 104);
    this->chargeAnim.setFrameRect(10, 1833, 15, 179, 108);
    this->chargeAnim.setFrameRect(11, 2016, 17, 179, 105);
    this->chargeAnim.setLoop(true);

    this->specialAnim.setTexture(&flyTex);
    this->specialAnim.setFrameCount(12);
    this->specialAnim.setFrameDelay(4);
    this->specialAnim.setFrameRect(0, 3, 2, 179, 121);
    this->specialAnim.setFrameRect(1, 186, 16, 179, 108);
    this->specialAnim.setFrameRect(2, 369, 18, 179, 105);
    this->specialAnim.setFrameRect(3, 552, 18, 179, 104);
    this->specialAnim.setFrameRect(4, 735, 15, 179, 108);
    this->specialAnim.setFrameRect(5, 918, 19, 179, 105);
    this->specialAnim.setFrameRect(6, 1101, 19, 179, 105);
    this->specialAnim.setFrameRect(7, 1284, 14, 179, 108);
    this->specialAnim.setFrameRect(8, 1467, 18, 179, 105);
    this->specialAnim.setFrameRect(9, 1650, 20, 179, 104);
    this->specialAnim.setFrameRect(10, 1833, 15, 179, 108);
    this->specialAnim.setFrameRect(11, 2016, 17, 179, 105);
    this->specialAnim.setLoop(false);

    this->sprite.setTexture(flyTex);
    this->sprite.setTextureRect(IntRect(3, 2, 179, 121));
    this->sprite.setScale(2.5f, 2.5f);
    this->switchAnim(&this->idleAnim);
    this->updateBoundingBox();
}

Hairbuster::~Hairbuster() {}

void Hairbuster::setFlyCenter(float cx, float cy) {
    this->flyCenterX = cx;
    this->flyCenterY = cy;
}

void Hairbuster::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }
        return;
    }
    if (player == nullptr) {
        return;
    }
    if (!this->entranceDone) {
        this->entranceTimer += 1.f / 60.f;
        if (this->entranceTimer >= 2.0f) {
            this->entranceDone = true;
        }
        this->flyAngle += this->flySpeed * 0.5f;
        this->position.x = this->flyCenterX + cosf(this->flyAngle) * this->flyRadiusX;
        this->position.y = this->flyCenterY + sinf(this->flyAngle) * this->flyRadiusY;
        this->faceRight = (sinf(this->flyAngle) < 0.f);
        this->switchAnim(&this->idleAnim);
        return;
    }

    if (this->bossPhase == 0 && this->getHealthFraction() <= this->phase2Threshold) {
        this->bossPhase = 1;
        this->flySpeed *= 1.6f;
        this->flyRadiusX *= 1.3f;
        this->flyRadiusY *= 1.2f;
        this->attackCooldown *= 0.6f;
        this->bombCooldown *= 0.5f;
        this->specialCooldown *= 0.6f;
    }

    float px = player->getPosition().x;
    this->faceRight = (px > this->position.x);

    if (this->isDiving) {
        this->diveTimer += 1.f / 60.f;
        if (this->diveTimer >= this->diveDuration) {
            this->isDiving = false;
            this->diveTimer = 0.f;
            float dx = this->position.x - this->flyCenterX;
            float dy = this->position.y - this->flyCenterY;
            this->flyAngle = atan2f(dy, dx);
        }
        else {
            float dx = this->diveTargetX - this->position.x;
            float dy = this->diveTargetY - this->position.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 10.f) {
                this->position.x += (dx / dist) * this->diveSpeed;
                this->position.y += (dy / dist) * this->diveSpeed;
            }
            this->faceRight = (dx > 0.f);
        }
        this->switchAnim(&this->chargeAnim);
        return;
    }

    this->flyAngle += this->flySpeed;
    this->position.x = this->flyCenterX + cosf(this->flyAngle) * this->flyRadiusX;
    this->position.y = this->flyCenterY + sinf(this->flyAngle) * this->flyRadiusY;

    float dist = this->distanceTo(player);

    bool doSpecial = this->specialTimer.getElapsedTime().asSeconds() >= this->specialCooldown;
    bool doDive = !doSpecial && this->chargeTimer.getElapsedTime().asSeconds() >= this->chargeCooldown;

    if (doDive) {
        this->isDiving = true;
        this->diveTimer = 0.f;
        this->diveTargetX = player->getPosition().x;
        this->diveTargetY = player->getPosition().y - 50.f;
        this->chargeTimer.restart();
        this->switchAnim(&this->chargeAnim);
    }
    else if (doSpecial) {
        this->aiState = AI_BOSS_SPECIAL;
        this->performAttack(player);
        this->aiState = AI_BOSS_IDLE;
        this->specialTimer.restart();
        this->switchAnim(&this->specialAnim);
    }
    else if (this->attackTimer.getElapsedTime().asSeconds() >= this->attackCooldown &&
        this->bombTimer.getElapsedTime().asSeconds() >= this->bombCooldown) {
        this->performAttack(player);
        this->attackTimer.restart();
        this->bombTimer.restart();
        this->switchAnim(&this->shootAnim);
    }
    else {
        this->switchAnim(&this->idleAnim);
    }
}

void Hairbuster::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) {
        return;
    }
    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = this->position;
    origin.y += (float)(this->frameH) * scaleX * 0.4f;
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    if (this->aiState == AI_BOSS_SPECIAL) {
        float angle = 70.f;
        this->pm->spawnExplosive(origin, dir, angle - 15.f, 5, 4, true);
        this->pm->spawnExplosive(origin, dir, angle, 5, 4, true);
        this->pm->spawnExplosive(origin, dir, angle + 15.f, 5, 4, true);

        if (this->bossPhase >= 1) {
            this->pm->spawnExplosive(origin, dir, angle - 30.f, 4, 3, true);
            this->pm->spawnExplosive(origin, dir, angle + 30.f, 4, 3, true);
        }
    }
    else {
        float angle = 65.f;
        this->pm->spawnExplosive(origin, dir, angle, 4, 3, true);

        if (this->bossPhase >= 1) {
            float shotAngle = 0.f;
            float dy = player->getPosition().y - this->position.y;
            float dx = player->getPosition().x - this->position.x;
            if (dx != 0.f || dy != 0.f) {
                shotAngle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
                if (shotAngle < -15.f) {
                    shotAngle = -15.f;
                }
                if (shotAngle > 45.f) {
                    shotAngle = 45.f;
                }
            }
            this->pm->spawnStraight(origin, dir, shotAngle, 3, true);
        }
    }
}

void Hairbuster::onDeath() {
    if (this->dying) {
        return;
    }
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->isCharging = false;
    this->isDiving = false;
    this->switchAnim(&this->deathAnim);
}

void Hairbuster::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

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
    float scale = 2.5f;
    if (this->dying) {
        IntRect texRect = this->sprite.getTextureRect();
        if (texRect.width > 200) {
            scale = 0.9f;
        }
    }
    else if (this->isDiving || this->isCharging) {
        scale = 2.8f;
    }

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    float drawY = this->position.y - scrollY;

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

void Hairbuster::handleCollision(Level* lvl) {
    this->onGround = false;
}

void Hairbuster::applyGravity() {
    this->velocityY = 0.f;
}

Ironokava::Ironokava(TextureManager* texMgr, AudioManager* audMgr) : Boss(texMgr, audMgr)
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
    this->frameW = 176;
    this->frameH = 120;
    this->baseFrameW = 176;
    this->baseFrameH = 120;
    this->walkFrames = 2;
    this->shootFrames = 2;
    this->deathFrames = 4;
    this->specialCooldown = 5.0f;
    this->chargeCooldown = 4.0f;
    this->chargeSpeed = 8.0f;
    this->chargeDuration = 1.2f;

    Texture& walkTex = texMgr->getTexture("resources/Sprites/ironokava.png");
    this->walkAnim.setTexture(&walkTex);
    this->walkAnim.setFrameCount(this->walkFrames);
    this->walkAnim.setFrameDelay(15);
    this->walkAnim.setFrameRect(0, 0, 0, 176, 120);
    this->walkAnim.setFrameRect(1, 180, 1, 175, 120);
    this->walkAnim.setLoop(true);

    this->shootAnim.setTexture(&walkTex);
    this->shootAnim.setFrameCount(this->shootFrames);
    this->shootAnim.setFrameDelay(8);
    this->shootAnim.setFrameRect(0, 0, 0, 176, 120);
    this->shootAnim.setFrameRect(1, 180, 1, 175, 120);
    this->shootAnim.setLoop(false);

    Texture& deathTex = texMgr->getTexture("resources/Sprites/ironokava-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(this->deathFrames);
    this->deathAnim.setFrameDelay(12);
    this->deathAnim.setFrameRect(0, 4, 95, 228, 150);
    this->deathAnim.setFrameRect(1, 232, 34, 222, 196);
    this->deathAnim.setFrameRect(2, 442, 40, 220, 199);
    this->deathAnim.setFrameRect(3, 672, 90, 199, 147);
    this->deathAnim.setLoop(false);

    this->idleAnim.setTexture(&walkTex);
    this->idleAnim.setFrameCount(this->walkFrames);
    this->idleAnim.setFrameDelay(25);
    this->idleAnim.setFrameRect(0, 0, 0, 176, 120);
    this->idleAnim.setFrameRect(1, 180, 1, 175, 120);
    this->idleAnim.setLoop(true);

    this->chargeAnim.setTexture(&walkTex);
    this->chargeAnim.setFrameCount(this->walkFrames);
    this->chargeAnim.setFrameDelay(4);
    this->chargeAnim.setFrameRect(0, 0, 0, 176, 120);
    this->chargeAnim.setFrameRect(1, 180, 1, 175, 120);
    this->chargeAnim.setLoop(true);

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
    if (this->pm == nullptr || player == nullptr) {
        return;
    }

    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = ProjectileManager::calcBarrelTip(
        this->position,
        this->faceRight ? DIR_RIGHT : DIR_LEFT,
        (float)(this->frameW) * scaleX,
        (float)(this->frameH) * 0.4f
    );

    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    if (this->aiState == AI_BOSS_SPECIAL) {
        float baseAngle = 0.f;
        float dy = player->getPosition().y - this->position.y;
        float dx = player->getPosition().x - this->position.x;
        if (dx != 0.f || dy != 0.f) {
            baseAngle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
            if (baseAngle < -20.f) {
                baseAngle = -20.f;
            }
            if (baseAngle > 45.f) {
                baseAngle = 45.f;
            }
        }

        this->pm->spawnExplosive(origin, dir, baseAngle - 15.f, 4, 3, true);
        this->pm->spawnExplosive(origin, dir, baseAngle, 4, 3, true);
        this->pm->spawnExplosive(origin, dir, baseAngle + 15.f, 4, 3, true);
    }
    else {
        float angle = 0.f;
        float dy = player->getPosition().y - this->position.y;
        float dx = player->getPosition().x - this->position.x;
        if (dx != 0.f || dy != 0.f) {
            angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
            if (angle < -15.f) {
                angle = -15.f;
            }
            if (angle > 45.f) {
                angle = 45.f;
            }
        }
        this->pm->spawnExplosive(origin, dir, angle, 3, 2, true);
        if (this->bossPhase >= 1) {
            this->pm->spawnStraight(origin, dir, angle, 3, true);
        }
    }
}

void Ironokava::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }
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
    float scale = 3.0f;
    if (this->dying) {
        IntRect texRect = this->sprite.getTextureRect();
        if (texRect.width > 180) {
            scale = 2.3f;
        }
    }
    else if (this->isCharging) {
        scale = 3.2f;
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

SeaSatan::SeaSatan(TextureManager* texMgr, AudioManager* audMgr)
    : Boss(texMgr, audMgr)
    , swimCenterX(9237.f)
    , surfaceY(1450.f)
    , floatBaseY(1450.f)
    , bobAngle(0.f), bobSpeed(0.025f), bobAmplitude(12.f)
    , patrolLeftX(8450.f), patrolRightX(10025.f)
    , moveDir(1.f), patrolSpeed(1.2f)
    , torpedoCooldown(2.5f), depthChargeCooldown(4.0f)
    , sonarPulseCooldown(6.0f), sonarPulseCount(0)
{
    this->setEnemyType(ENEMY_BOSS_SEASATAN);
    this->bossName = "SEASATAN";
    this->maxHealth = 150;
    this->currentHP = 150;
    this->health = 150;
    this->scoreValue = 10000;
    this->detectionRange = 3000.f;
    this->attackRange = 1500.f;
    this->attackCooldown = 2.0f;
    this->maxVelocity = 3.0f;
    this->baseMaxVelocity = 3.0f;
    this->deathDuration = 4.0f;
    this->deathSpriteScale = 1.0f;
    this->onGround = false;
    this->frameW = 364;
    this->frameH = 332;
    this->baseFrameW = 364;
    this->baseFrameH = 243;
    this->walkFrames = 4;
    this->shootFrames = 4;
    this->deathFrames = 3;
    this->specialCooldown = 5.0f;
    this->chargeCooldown = 8.0f;
    this->chargeSpeed = 5.0f;
    this->chargeDuration = 2.0f;
    this->phase2Threshold = 0.4f;

    Texture& swimTex = texMgr->getTexture("resources/Sprites/seasatan.png");
    this->swimAnim.setTexture(&swimTex);
    this->swimAnim.setFrameCount(4);
    this->swimAnim.setFrameDelay(8);
    this->swimAnim.setFrameRect(0, 15, 53, 364, 332);
    this->swimAnim.setFrameRect(1, 394, 53, 364, 243);
    this->swimAnim.setFrameRect(2, 773, 53, 364, 248);
    this->swimAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->swimAnim.setLoop(true);

    this->walkAnim.setTexture(&swimTex);
    this->walkAnim.setFrameCount(4);
    this->walkAnim.setFrameDelay(10);
    this->walkAnim.setFrameRect(0, 15, 53, 364, 332);
    this->walkAnim.setFrameRect(1, 394, 53, 364, 243);
    this->walkAnim.setFrameRect(2, 773, 53, 364, 248);
    this->walkAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->walkAnim.setLoop(true);

    this->shootAnim.setTexture(&swimTex);
    this->shootAnim.setFrameCount(4);
    this->shootAnim.setFrameDelay(4);
    this->shootAnim.setFrameRect(0, 15, 53, 364, 332);
    this->shootAnim.setFrameRect(1, 394, 53, 364, 243);
    this->shootAnim.setFrameRect(2, 773, 53, 364, 248);
    this->shootAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->shootAnim.setLoop(false);

    Texture& deathTex = texMgr->getTexture("resources/Sprites/seasatan-death.png");
    this->deathAnim.setTexture(&deathTex);
    this->deathAnim.setFrameCount(3);
    this->deathAnim.setFrameDelay(18);
    this->deathAnim.setFrameRect(0, 15, 30, 805, 514);
    this->deathAnim.setFrameRect(1, 36, 552, 796, 426);
    this->deathAnim.setFrameRect(2, 912, 515, 595, 453);
    this->deathAnim.setLoop(false);

    this->idleAnim.setTexture(&swimTex);
    this->idleAnim.setFrameCount(4);
    this->idleAnim.setFrameDelay(12);
    this->idleAnim.setFrameRect(0, 15, 53, 364, 332);
    this->idleAnim.setFrameRect(1, 394, 53, 364, 243);
    this->idleAnim.setFrameRect(2, 773, 53, 364, 248);
    this->idleAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->idleAnim.setLoop(true);

    this->chargeAnim.setTexture(&swimTex);
    this->chargeAnim.setFrameCount(4);
    this->chargeAnim.setFrameDelay(3);
    this->chargeAnim.setFrameRect(0, 15, 53, 364, 332);
    this->chargeAnim.setFrameRect(1, 394, 53, 364, 243);
    this->chargeAnim.setFrameRect(2, 773, 53, 364, 248);
    this->chargeAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->chargeAnim.setLoop(true);

    this->specialAnim.setTexture(&swimTex);
    this->specialAnim.setFrameCount(4);
    this->specialAnim.setFrameDelay(5);
    this->specialAnim.setFrameRect(0, 15, 53, 364, 332);
    this->specialAnim.setFrameRect(1, 394, 53, 364, 243);
    this->specialAnim.setFrameRect(2, 773, 53, 364, 248);
    this->specialAnim.setFrameRect(3, 1152, 53, 364, 242);
    this->specialAnim.setLoop(false);

    this->sprite.setTexture(swimTex);
    this->sprite.setTextureRect(IntRect(15, 53, 364, 332));
    this->sprite.setScale(1.2f, 1.2f);
    this->switchAnim(&this->swimAnim);
    this->updateBoundingBox();
}

SeaSatan::~SeaSatan() {}

void SeaSatan::setSwimCenter(float cx, float cy, float surfY) {
    this->swimCenterX = cx;
    this->surfaceY = surfY;
    this->floatBaseY = surfY;
    this->patrolLeftX = 8450.f;
    this->patrolRightX = 10025.f;
}

void SeaSatan::applyGravity() {

    this->velocityY = 0.f;
}

void SeaSatan::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }
        return;
    }
    if (player == nullptr) {
        return;
    }

    if (!this->entranceDone) {
        this->entranceTimer += 1.f / 60.f;
        float progress = this->entranceTimer / 2.0f;
        if (progress > 1.f) {
            progress = 1.f;
        }
        this->position.y = this->floatBaseY + 300.f * (1.f - progress);
        this->position.x = this->swimCenterX;
        this->faceRight = (player->getPosition().x > this->position.x);
        this->switchAnim(&this->swimAnim);
        if (this->entranceTimer >= 2.0f) {
            this->entranceDone = true;
            this->position.y = this->floatBaseY;
        }
        return;
    }

    if (this->bossPhase == 0 && this->getHealthFraction() <= this->phase2Threshold) {
        this->bossPhase = 1;
        this->patrolSpeed *= 1.6f;
        this->bobSpeed *= 1.4f;
        this->bobAmplitude *= 1.3f;
        this->attackCooldown *= 0.5f;
        this->torpedoCooldown *= 0.5f;
        this->depthChargeCooldown *= 0.6f;
        this->specialCooldown *= 0.5f;
    }

    float px = player->getPosition().x;
    this->faceRight = (px > this->position.x);

    this->position.x += this->moveDir * this->patrolSpeed;
    if (this->position.x >= this->patrolRightX) {
        this->position.x = this->patrolRightX;
        this->moveDir = -1.f;
    }
    else if (this->position.x <= this->patrolLeftX) {
        this->position.x = this->patrolLeftX;
        this->moveDir = 1.f;
    }

    this->bobAngle += this->bobSpeed;
    this->position.y = this->floatBaseY + sinf(this->bobAngle) * this->bobAmplitude;

    if (this->isCharging) {
        this->chargeElapsed += 1.f / 60.f;
        float dx = px - this->position.x;
        float dir = (dx > 0.f) ? 1.f : -1.f;
        this->position.x += dir * this->chargeSpeed;

        if (this->chargeElapsed >= this->chargeDuration) {
            this->isCharging = false;
            this->chargeElapsed = 0.f;
        }
        this->switchAnim(&this->chargeAnim);
        return;
    }

    bool doSpecial = this->specialTimer.getElapsedTime().asSeconds() >= this->specialCooldown;
    bool doCharge = !doSpecial && this->chargeTimer.getElapsedTime().asSeconds() >= this->chargeCooldown;

    if (doCharge) {
        this->isCharging = true;
        this->chargeElapsed = 0.f;
        this->chargeTimer.restart();
        this->switchAnim(&this->chargeAnim);
    }
    else if (doSpecial) {
        this->aiState = AI_BOSS_SPECIAL;
        this->performAttack(player);
        this->aiState = AI_BOSS_IDLE;
        this->specialTimer.restart();
        this->switchAnim(&this->specialAnim);
    }
    else if (this->attackTimer.getElapsedTime().asSeconds() >= this->attackCooldown &&
        this->torpedoTimer.getElapsedTime().asSeconds() >= this->torpedoCooldown) {
        this->performAttack(player);
        this->attackTimer.restart();
        this->torpedoTimer.restart();
        this->switchAnim(&this->shootAnim);
    }
    else {
        this->switchAnim(&this->swimAnim);
    }
}

void SeaSatan::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) {
        return;
    }
    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = this->position;
    origin.y += (float)(this->frameH) * scaleX * 0.3f;
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    if (this->aiState == AI_BOSS_SPECIAL) {

        float baseAngle = 60.f;
        this->pm->spawnExplosive(origin, dir, baseAngle - 30.f, 5, 4, true);
        this->pm->spawnExplosive(origin, dir, baseAngle - 15.f, 5, 4, true);
        this->pm->spawnExplosive(origin, dir, baseAngle, 6, 5, true);
        this->pm->spawnExplosive(origin, dir, baseAngle + 15.f, 5, 4, true);
        this->pm->spawnExplosive(origin, dir, baseAngle + 30.f, 5, 4, true);

        if (this->bossPhase >= 1) {

            float angle = 0.f;
            float dy = player->getPosition().y - this->position.y;
            float dx = player->getPosition().x - this->position.x;
            if (dx != 0.f || dy != 0.f) {
                angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
                if (angle < -20.f) {
                    angle = -20.f;
                }
                if (angle > 45.f) {
                    angle = 45.f;
                }
            }
            this->pm->spawnStraight(origin, dir, angle, 4, true);
            this->pm->spawnStraight(origin, dir, angle + 10.f, 3, true);
            this->pm->spawnStraight(origin, dir, angle - 10.f, 3, true);
        }
    }
    else {

        float dy = player->getPosition().y - this->position.y;
        float dx = player->getPosition().x - this->position.x;
        float angle = 50.f;
        if (dx != 0.f || dy != 0.f) {
            angle = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
            if (angle < 15.f) {
                angle = 15.f;
            }
            if (angle > 75.f) {
                angle = 75.f;
            }
        }

        this->pm->spawnExplosive(origin, dir, angle, 4, 3, true);

        if (this->bossPhase >= 1) {

            this->pm->spawnExplosive(origin, dir, angle + 10.f, 4, 3, true);
            this->pm->spawnStraight(origin, dir, angle - 5.f, 3, true);
        }
    }
}

void SeaSatan::onDeath() {
    if (this->dying) {
        return;
    }
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->isCharging = false;
    this->switchAnim(&this->deathAnim);
}

void SeaSatan::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

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

    float scale = 1.2f;
    if (this->dying) {
        IntRect texRect = this->sprite.getTextureRect();
        if (texRect.width > 400) {
            scale = 0.7f;
        }
        else {
            scale = 0.9f;
        }
    }
    else if (this->isCharging) {
        scale = 1.4f;
    }

    if (this->faceRight) {
        this->sprite.setScale(-scale, scale);
    }
    else {
        this->sprite.setScale(scale, scale);
    }

    float drawY = this->position.y - scrollY;
    if (this->dying) {
        drawY += 40.f;
    }

    this->sprite.setPosition(this->position.x - scrollX, drawY);
    window.draw(this->sprite);
}

void SeaSatan::handleCollision(Level* lvl) {

    this->onGround = false;
}

// ============================================================
// SHERRY BOSS IMPLEMENTATION
// Multi-phase boss: Throne -> Getting Up -> Stood Up -> Walking
// Attacks: Laser eyes + random missile bursts
// ============================================================

Sherry::Sherry(TextureManager* texMgr, AudioManager* audMgr)
    : Boss(texMgr, audMgr)
    , sherryPhase(0)
    , phaseTimer(0.f)
    , throneX(0.f)
    , throneY(0.f)
    , throneVisible(false)
    , emptyThroneTex(nullptr)
    , laserCooldown(3.0f)
    , missileCooldown(5.0f)
    , missileBurstCooldown(4.0f)
    , missileBurstCount(0)
    , missileBurstMax(5)
    , missileBurstInterval(0.15f)
    , inMissileBurst(false)
    , isFiringLaser(false)
    , laserDuration(1.5f)
    , laserElapsed(0.f)
    , laserAngle(0.f)
    , laserDamageApplied(false)
    , patrolLeftX(0.f)
    , patrolRightX(0.f)
    , patrolDir(-1.f)
{
    this->setEnemyType(ENEMY_BOSS_SHERRY);
    this->bossName = "SIR SHEHRYAR";
    this->maxHealth = 250;
    this->currentHP = 250;
    this->health = 250;
    this->scoreValue = 15000;
    this->detectionRange = 3000.f;
    this->attackRange = 1200.f;
    this->attackCooldown = 1.5f;
    this->maxVelocity = 2.5f;
    this->baseMaxVelocity = 2.5f;
    this->deathDuration = 3.5f;
    this->deathSpriteScale = 1.0f;
    this->onGround = true;
    this->entranceDone = true; // We handle our own entrance via sherryPhase
    this->frameW = 228;
    this->frameH = 506;
    this->baseFrameW = 228;
    this->baseFrameH = 506;
    this->walkFrames = 5;
    this->shootFrames = 5;
    this->deathFrames = 5;
    this->specialCooldown = 3.0f;
    this->chargeCooldown = 8.0f;
    this->chargeSpeed = 6.0f;
    this->chargeDuration = 1.5f;
    this->phase2Threshold = 0.4f;

    // --- Throne sitting animation (1 frame) ---
    Texture& throneTex = texMgr->getTexture("resources/Sprites/sherry-onthrone.png");
    this->throneAnim.setTexture(&throneTex);
    this->throneAnim.setFrameCount(1);
    this->throneAnim.setFrameDelay(1);
    this->throneAnim.setFrameRect(0, 158, 62, 707, 1206);
    this->throneAnim.setLoop(true);

    // --- Getting up animation (4 frames) ---
    Texture& getupTex = texMgr->getTexture("resources/Sprites/sherry-gettingup.png");
    this->getupAnim.setTexture(&getupTex);
    this->getupAnim.setFrameCount(4);
    this->getupAnim.setFrameDelay(12);
    this->getupAnim.setFrameRect(0, 14, 272, 296, 568);
    this->getupAnim.setFrameRect(1, 424, 269, 299, 571);
    this->getupAnim.setFrameRect(2, 840, 271, 288, 570);
    this->getupAnim.setFrameRect(3, 1215, 271, 299, 569);
    this->getupAnim.setLoop(false);

    // --- Stood up static (1 frame) ---
    Texture& stoodupTex = texMgr->getTexture("resources/Sprites/sherry-stoodup.png");
    this->stoodupAnim.setTexture(&stoodupTex);
    this->stoodupAnim.setFrameCount(1);
    this->stoodupAnim.setFrameDelay(1);
    this->stoodupAnim.setFrameRect(0, 90, 56, 228, 506);
    this->stoodupAnim.setLoop(true);

    // --- Walking animation (5 frames) ---
    Texture& walkTex = texMgr->getTexture("resources/Sprites/sherry-walk.png");
    this->sherryWalkAnim.setTexture(&walkTex);  // use Sherry-specific member
    this->sherryWalkAnim.setFrameCount(5);
    this->sherryWalkAnim.setFrameDelay(8);
    this->sherryWalkAnim.setFrameRect(0, 8, 482, 237, 503);
    this->sherryWalkAnim.setFrameRect(1, 246, 495, 172, 489);
    this->sherryWalkAnim.setFrameRect(2, 440, 481, 162, 504);
    this->sherryWalkAnim.setFrameRect(3, 608, 473, 178, 512);
    this->sherryWalkAnim.setFrameRect(4, 792, 479, 227, 506);
    this->sherryWalkAnim.setLoop(true);

    // --- Empty throne sprite ---
    Texture& emptyThroneTexture = texMgr->getTexture("resources/Sprites/empty-throne.png");
    this->emptyThroneTex = &emptyThroneTexture;
    this->emptyThroneSprite.setTexture(emptyThroneTexture);
    this->emptyThroneSprite.setTextureRect(IntRect(255, 221, 510, 947));

    // --- Death animation (reuse walk as placeholder - you can add a death sprite) ---
    this->deathAnim.setTexture(&walkTex);
    this->deathAnim.setFrameCount(5);
    this->deathAnim.setFrameDelay(10);
    this->deathAnim.setFrameRect(0, 8, 482, 237, 503);
    this->deathAnim.setFrameRect(1, 246, 495, 172, 489);
    this->deathAnim.setFrameRect(2, 440, 481, 162, 504);
    this->deathAnim.setFrameRect(3, 608, 473, 178, 512);
    this->deathAnim.setFrameRect(4, 792, 479, 227, 506);
    this->deathAnim.setLoop(false);

    // --- Boss base class animations (used in walking phase) ---
    this->idleAnim.setTexture(&walkTex);
    this->idleAnim.setFrameCount(5);
    this->idleAnim.setFrameDelay(10);
    this->idleAnim.setFrameRect(0, 8, 482, 237, 503);
    this->idleAnim.setFrameRect(1, 246, 495, 172, 489);
    this->idleAnim.setFrameRect(2, 440, 481, 162, 504);
    this->idleAnim.setFrameRect(3, 608, 473, 178, 512);
    this->idleAnim.setFrameRect(4, 792, 479, 227, 506);
    this->idleAnim.setLoop(true);

    this->shootAnim.setTexture(&walkTex);
    this->shootAnim.setFrameCount(5);
    this->shootAnim.setFrameDelay(4);
    this->shootAnim.setFrameRect(0, 8, 482, 237, 503);
    this->shootAnim.setFrameRect(1, 246, 495, 172, 489);
    this->shootAnim.setFrameRect(2, 440, 481, 162, 504);
    this->shootAnim.setFrameRect(3, 608, 473, 178, 512);
    this->shootAnim.setFrameRect(4, 792, 479, 227, 506);
    this->shootAnim.setLoop(false);

    this->chargeAnim.setTexture(&walkTex);
    this->chargeAnim.setFrameCount(5);
    this->chargeAnim.setFrameDelay(3);
    this->chargeAnim.setFrameRect(0, 8, 482, 237, 503);
    this->chargeAnim.setFrameRect(1, 246, 495, 172, 489);
    this->chargeAnim.setFrameRect(2, 440, 481, 162, 504);
    this->chargeAnim.setFrameRect(3, 608, 473, 178, 512);
    this->chargeAnim.setFrameRect(4, 792, 479, 227, 506);
    this->chargeAnim.setLoop(true);

    this->specialAnim.setTexture(&walkTex);
    this->specialAnim.setFrameCount(5);
    this->specialAnim.setFrameDelay(4);
    this->specialAnim.setFrameRect(0, 8, 482, 237, 503);
    this->specialAnim.setFrameRect(1, 246, 495, 172, 489);
    this->specialAnim.setFrameRect(2, 440, 481, 162, 504);
    this->specialAnim.setFrameRect(3, 608, 473, 178, 512);
    this->specialAnim.setFrameRect(4, 792, 479, 227, 506);
    this->specialAnim.setLoop(false);

    // --- Set initial sprite to throne ---
    this->sprite.setTexture(throneTex);
    this->sprite.setTextureRect(IntRect(158, 62, 707, 1206));
    this->sprite.setScale(0.35f, 0.35f);  // PHASE 0 INITIAL SCALE (throne sitting)
    this->switchAnim(&this->throneAnim);
    this->updateBoundingBox();
}

Sherry::~Sherry() {}

void Sherry::setThronePosition(float x, float y) {
    this->throneX = x;
    this->throneY = y;
    this->patrolLeftX = x - 400.f;
    this->patrolRightX = x + 400.f;
}

void Sherry::applyGravity() {
    // Sherry stays on the ground in all phases
    this->velocityY = 0.f;
}

void Sherry::updateAI(PlayerSoldier* player, Level* lvl) {
    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
        }
        return;
    }
    if (player == nullptr) {
        return;
    }

    float dt = 1.f / 60.f;

    // =============================================
    // Phase 0: Sitting on throne for 2 seconds
    // =============================================
    if (this->sherryPhase == 0) {
        this->velocityX = 0.f;
        this->switchAnim(&this->throneAnim);

        if (player != nullptr) {
            this->faceRight = (player->getPosition().x > this->position.x);

            // Start timer only when player is within range
            float dist = this->distanceTo(player);
            if (dist <= 500.f) {
                this->phaseTimer += dt;
            }
            // Get up after timer reaches 2 seconds
            if (this->phaseTimer >= 2.0f) {
                this->sherryPhase = 1;
                this->phaseTimer = 0.f;
                this->switchAnim(&this->getupAnim);
            }
        }
        return;
    }

    // =============================================
    // Phase 1: Getting up animation (4 frames, non-looping)
    // =============================================
    if (this->sherryPhase == 1) {
        this->velocityX = 0.f;

        if (this->getupAnim.isFinished()) {
            // Getting up done -> skip stood up phase, go directly to walking
            this->sherryPhase = 3;
            this->phaseTimer = 0.f;
            this->throneVisible = true;
            // Start walking from beside the throne (to the left)
            this->position.x = this->throneX - 400.f;
            // Update frame dimensions for walking
            this->frameW = 237;
            this->frameH = 503;
            this->baseFrameW = 237;
            this->baseFrameH = 503;
            this->switchAnim(&this->sherryWalkAnim);
            this->updateBoundingBox();
        }
        return;
    }

    // =============================================
    // Phase 3: Walking + Attacks (laser eyes + missile bursts)
    // Empty throne remains visible at original position
    // =============================================
    if (this->sherryPhase == 3) {
        // Boss phase 2 (enraged) when HP drops below threshold
        if (this->bossPhase == 0 && this->getHealthFraction() <= this->phase2Threshold) {
            this->bossPhase = 1;
            this->maxVelocity = this->baseMaxVelocity * 1.5f;
            this->attackCooldown *= 0.6f;
            this->laserCooldown *= 0.6f;
            this->missileBurstCooldown *= 0.7f;
            this->missileBurstMax = 7;
        }

        // Face the player
        float px = player->getPosition().x;
        this->faceRight = (px > this->position.x);

        // Handle laser firing
        if (this->isFiringLaser) {
            this->laserElapsed += dt;
            if (this->laserElapsed >= this->laserDuration) {
                this->isFiringLaser = false;
                this->laserElapsed = 0.f;
                this->laserDamageApplied = false;
            }
            // Apply laser damage once per laser activation if player is in the beam path
            if (!this->laserDamageApplied) {
                float lpx = player->getPosition().x;
                float lpy = player->getPosition().y;
                // Check if player is on the correct side (in front of boss)
                bool inFront = (this->faceRight && lpx > this->position.x) ||
                    (!this->faceRight && lpx < this->position.x);
                float dist = fabsf(lpx - this->position.x);
                if (inFront && dist < 1200.f && fabsf(lpy - this->position.y) < 200.f) {
                    player->takeDamage(1);
                    // Phase 2: extra damage
                    if (this->bossPhase >= 1) {
                        player->takeDamage(1);
                    }
                }
                this->laserDamageApplied = true;
            }
            this->velocityX = 0.f;
            this->switchAnim(&this->shootAnim);
            return;
        }

        // Handle missile burst (rapid fire missiles)
        if (this->inMissileBurst) {
            if (this->missileBurstClock.getElapsedTime().asSeconds() >= this->missileBurstInterval) {
                this->performAttack(player);
                this->missileBurstCount++;
                this->missileBurstClock.restart();

                if (this->missileBurstCount >= this->missileBurstMax) {
                    this->inMissileBurst = false;
                    this->missileBurstCount = 0;
                    this->missileBurstTimer.restart();
                }
            }
            // Slow walk during burst
            this->position.x += this->patrolDir * this->maxVelocity * 0.3f;
            this->switchAnim(&this->sherryWalkAnim);
            return;
        }

        // Decide attacks
        bool doLaser = this->laserTimer.getElapsedTime().asSeconds() >= this->laserCooldown;
        bool doMissileBurst = !doLaser && this->missileBurstTimer.getElapsedTime().asSeconds() >= this->missileBurstCooldown;

        if (doLaser) {
            // Fire laser from eyes aimed at the player
            this->isFiringLaser = true;
            this->laserElapsed = 0.f;
            this->laserDamageApplied = false;
            this->laserTimer.restart();
            this->aiState = AI_BOSS_SPECIAL;

            // Calculate laser angle toward the player (for visual beam in draw())
            float scaleX = std::abs(this->sprite.getScale().x);
            sf::Vector2f origin = this->position;
            origin.y += (float)(this->frameH) * scaleX * 0.15f; // eye level
            if (this->faceRight) {
                origin.x += (float)(this->frameW) * scaleX * 0.5f;
            }
            else {
                origin.x -= (float)(this->frameW) * scaleX * 0.5f;
            }

            float dx = player->getPosition().x - this->position.x;
            float dy = player->getPosition().y - origin.y;
            float angleTowardPlayer = 0.f;
            if (dx != 0.f || dy != 0.f) {
                angleTowardPlayer = atan2f(-dy, fabsf(dx)) * 180.f / 3.14159f;
                if (angleTowardPlayer < -30.f) angleTowardPlayer = -30.f;
                if (angleTowardPlayer > 45.f) angleTowardPlayer = 45.f;
            }
            this->laserAngle = angleTowardPlayer;

            this->velocityX = 0.f;
            this->switchAnim(&this->specialAnim);
            return;
        }
        else if (doMissileBurst) {
            // Start missile burst
            this->inMissileBurst = true;
            this->missileBurstCount = 0;
            this->missileBurstClock.restart();
            this->aiState = AI_BOSS_ATTACK;
            return;
        }

        // Normal walking / patrol behavior
        this->aiState = AI_BOSS_WALK;
        this->switchAnim(&this->sherryWalkAnim);

        // Patrol between bounds
        this->position.x += this->patrolDir * this->maxVelocity;
        if (this->position.x <= this->patrolLeftX) {
            this->position.x = this->patrolLeftX;
            this->patrolDir = 1.f;
        }
        else if (this->position.x >= this->patrolRightX) {
            this->position.x = this->patrolRightX;
            this->patrolDir = -1.f;
        }

        // Also chase player if far
        if (fabsf(px - this->position.x) > 300.f) {
            if (px > this->position.x + 20.f) {
                this->velocityX += 0.5f;
                if (this->velocityX > this->maxVelocity) {
                    this->velocityX = this->maxVelocity;
                }
            }
            else if (px < this->position.x - 20.f) {
                this->velocityX -= 0.5f;
                if (this->velocityX < -this->maxVelocity) {
                    this->velocityX = -this->maxVelocity;
                }
            }
        }
    }
}

void Sherry::performAttack(PlayerSoldier* player) {
    if (this->pm == nullptr || player == nullptr) {
        return;
    }

    // Missile attack (parabolic path with missile.png)
    float scaleX = std::abs(this->sprite.getScale().x);
    sf::Vector2f origin = this->position;
    origin.y += (float)(this->frameH) * scaleX * 0.3f;
    // Offset X origin to the shoulder on the facing side
    if (this->faceRight) {
        origin.x += (float)(this->frameW) * scaleX * 0.3f;
    }
    else {
        origin.x -= (float)(this->frameW) * scaleX * 0.3f;
    }
    int dir = this->faceRight ? DIR_RIGHT : DIR_LEFT;

    // Calculate distance to player for trajectory scaling
    float dx = fabsf(player->getPosition().x - this->position.x);
    float dy = player->getPosition().y - this->position.y;

    // Scale the launch angle and speed based on distance
    // Closer = steeper angle, slower; Farther = flatter angle, faster
    float angle = 45.f;
    float missileSpeed = 7.f;

    if (dx > 0.f || dy != 0.f) {
        angle = atan2f(-dy, dx) * 180.f / 3.14159f;
        // Wider angle range for bigger parabolic arcs
        if (angle < 10.f) {
            angle = 10.f;
        }
        if (angle > 75.f) {
            angle = 75.f;
        }
    }

    // Increase speed proportionally to distance for longer range
    // Base speed 7 at close range, up to 14 at far range
    missileSpeed = 7.f + (dx / 200.f) * 2.f;
    if (missileSpeed > 14.f) missileSpeed = 14.f;
    if (missileSpeed < 7.f) missileSpeed = 7.f;

    // Add higher arc: boost the angle for longer distances to create bigger parabola
    if (dx > 300.f) {
        angle += 10.f;  // Extra arc for medium range
    }
    if (dx > 600.f) {
        angle += 10.f;  // Extra arc for long range
    }
    if (angle > 80.f) angle = 80.f;

    // Add some randomness to missile direction for burst effect
    float randomOffset = (float)(std::rand() % 20 - 10);
    this->pm->spawnMissile(origin, dir, angle + randomOffset, 2, 3, true, missileSpeed);

    // Phase 2: extra missile during burst
    if (this->bossPhase >= 1 && this->missileBurstCount % 2 == 0) {
        float missileAngle = angle + randomOffset - 10.f;
        this->pm->spawnMissile(origin, dir, missileAngle, 1, 2, true, missileSpeed + 2.f);
    }
}

void Sherry::onDeath() {
    if (this->dying) {
        return;
    }
    this->dying = true;
    this->deathTimer.restart();
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->isCharging = false;
    this->isFiringLaser = false;
    this->inMissileBurst = false;
    this->switchAnim(&this->deathAnim);
}

void Sherry::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) {
        return;
    }

    if (this->dying) {
        if (this->deathTimer.getElapsedTime().asSeconds() >= this->deathDuration) {
            this->status = false;
            return;
        }
    }

    // --- Draw empty throne first (behind the boss) ---
    if (this->throneVisible) {
        float emptyThroneScale = 0.45f;                          // EMPTY THRONE SCALE
        float emptyThroneOffsetX = -240.f;                         // EMPTY THRONE X offset from throneX
        float emptyThroneOffsetY = -60.f;                         // EMPTY THRONE Y offset from throneY
        this->emptyThroneSprite.setScale(emptyThroneScale, emptyThroneScale);
        this->emptyThroneSprite.setPosition(
            this->throneX + emptyThroneOffsetX - scrollX,
            this->throneY + emptyThroneOffsetY - scrollY
        );
        window.draw(this->emptyThroneSprite);
    }

    // ========================================================
    // PER-PHASE SCALE AND POSITION CONTROLS
    // Each phase has its own scale, Y offset, and X offset.
    // Adjust these values to fine-tune each phase's look.
    // ========================================================

    // Phase 0 defaults (overridden below per-phase)
    float scale = 0.5f;                    // PHASE 2/3 DEFAULT SCALE (stood up / walking)
    float drawY = this->position.y - scrollY;  // GLOBAL Y OFFSET (-20 = 20px up)
    float drawX = this->position.x - scrollX;         // DEFAULT X POSITION

    if (this->dying) {
        scale = 0.4f;                      // DYING SCALE
        drawY += 50.f;                     // DYING Y offset (sink down)
    }
    else if (this->isFiringLaser) {
        scale = 0.55f;                     // LASER FIRING SCALE
    }
    else if (this->isCharging) {
        scale = 0.6f;                      // CHARGING SCALE
    }

    // Phase 0: throne sprite (boss sitting on throne)
    if (this->sherryPhase == 0) {
        float throneSpriteScale = 0.35f;   // PHASE 0 SCALE (sitting on throne)
        float throneOffsetX = 0.f;         // PHASE 0 X offset from position.x
        float throneOffsetY = -70.f;         // PHASE 0 Y offset from position.y
        if (this->currentAnim != nullptr) {
            this->currentAnim->update();
            this->currentAnim->applyToSprite(this->sprite);
        }
        if (this->faceRight) {
            this->sprite.setScale(throneSpriteScale, throneSpriteScale);
        }
        else {
            this->sprite.setScale(-throneSpriteScale, throneSpriteScale);
        }
        this->sprite.setPosition(drawX + throneOffsetX, drawY + throneOffsetY);
        window.draw(this->sprite);
        return;
    }

    // Phase 1: getting up animation
    if (this->sherryPhase == 1) {
        float getupScale = 0.8f;           // PHASE 1 SCALE (getting up)
        float getupOffsetX = 0.f;          // PHASE 1 X offset from position.x
        float getupOffsetY = -60.f;          // PHASE 1 Y offset from position.y
        if (this->currentAnim != nullptr) {
            this->currentAnim->update();
            this->currentAnim->applyToSprite(this->sprite);
        }
        if (this->faceRight) {
            this->sprite.setScale(getupScale, getupScale);
        }
        else {
            this->sprite.setScale(-getupScale, getupScale);
        }
        this->sprite.setPosition(drawX + getupOffsetX, drawY + getupOffsetY);
        window.draw(this->sprite);
        return;
    }

    // Phase 2 & 3: stood up / walking
    float phase23OffsetX = 0.f;           // PHASE 2/3 X offset from position.x
    float phase23OffsetY = 60.f;           // PHASE 2/3 Y offset from drawY
    if (this->currentAnim != nullptr) {
        this->currentAnim->update();
        this->currentAnim->applyToSprite(this->sprite);
    }

    // Use Sherry's walk anim in phase 3
    if (this->sherryPhase == 3 && !this->dying && !this->isFiringLaser) {
        this->sherryWalkAnim.update();
        this->sherryWalkAnim.applyToSprite(this->sprite);
    }

    {
        float nscale = 0.6f;
        if (this->faceRight) {
            this->sprite.setScale(nscale, nscale);
        }
        else {
            this->sprite.setScale(-nscale, nscale);
        }
    }

    // Apply phase 2/3 position offset
    drawX += phase23OffsetX;
    drawY += phase23OffsetY;

    // Laser visual effect (glow at eye level when firing)
    if (this->isFiringLaser && !this->dying) {
        float scaleX = std::abs(this->sprite.getScale().x);
        float eyeX = this->position.x - scrollX;
        float eyeY = drawY + (float)(this->frameH) * scale * 0.15f;
        if (this->faceRight) {
            eyeX += (float)(this->frameW) * scale * 0.5f;
        }
        else {
            eyeX -= (float)(this->frameW) * scale * 0.5f;
        }

        // Draw laser beam line angled toward player
        float beamLen = 2000.f;
        float angleRad = this->laserAngle * 3.14159f / 180.f;
        float dirSign = this->faceRight ? 1.f : -1.f;

        float endX = eyeX + dirSign * beamLen * cosf(angleRad);
        float endY = eyeY - beamLen * sinf(angleRad);

        // Use a rotated rectangle for the laser beam
        sf::RectangleShape laserBeam(sf::Vector2f(beamLen, 8.f));
        laserBeam.setFillColor(sf::Color(255, 50, 50, 200));
        laserBeam.setOrigin(0.f, 4.f);
        laserBeam.setPosition(eyeX, eyeY);
        // Fix rotation based on facing direction
        // Right: -laserAngle tilts beam upward from rightward direction
        // Left: 180+laserAngle points beam leftward and tilts upward
        if (this->faceRight) {
            laserBeam.setRotation(-this->laserAngle);
        }
        else {
            laserBeam.setRotation(180.f + this->laserAngle);
        }
        window.draw(laserBeam);

        // Glow effect at eye
        sf::CircleShape glow(20.f);
        glow.setFillColor(sf::Color(255, 100, 100, 180));
        glow.setPosition(eyeX - 20.f, eyeY - 20.f);
        window.draw(glow);
    }

    this->sprite.setPosition(drawX, drawY);
    window.draw(this->sprite);
}

void Sherry::handleCollision(Level* lvl) {
    if (lvl == nullptr) {
        return;
    }
    if (this->dying) {
        return;
    }
    // In throne/getup/stoodup phases, don't move
    if (this->sherryPhase < 3) {
        this->onGround = true;
        return;
    }
    // In walking phase, use normal collision
    Enemy::handleCollision(lvl);
}
