#include "Soldier.h"
#include "Level.h"

Soldier::Soldier(TextureManager* texMgr, AudioManager* audMgr)
    : DamagableEntity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , maxVelocity(5.f)
    , baseMaxVelocity(5.f)
    , direction(DIR_RIGHT)
    , onGround(false)
    , lives(3)
    , currentHP(3)
    , saturation(100)
    , meleeDamage(1.f)
    , meleeCooldown(0.5f)
    , transformState(nullptr)
{
}

Soldier::~Soldier() {
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
    }
    this->transformState = nullptr;
}

void Soldier::update(float scroll, Level* lvl) {
    this->handleStateTimers();
    this->applyGravity();
    this->applyMovement(scroll);
    this->handleCollision(lvl);
}

void Soldier::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (this->velocityX != 0.f || !this->onGround) {
        this->animation.update();
    }
    else {
        this->animation.currentFrame = 5;
        this->animation.clock.restart();
    }
    this->animation.applyToSprite(this->sprite);
    this->sprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(this->sprite);
}

void Soldier::takeDamage(int amount) {
    
    
    
    
    
    if (amount < 0) return;

    if (this->transformState != nullptr) {
        
    }

    this->currentHP -= amount;
    if (this->currentHP < 0) this->currentHP = 0;

    if (this->currentHP == 0) {
        this->onDeath();
    }
    else if (this->currentHP == 1) {
        
    }
    else if (this->currentHP == 2) {
        
    }

    
    
    this->health = this->currentHP;
}

void Soldier::meleeAttack() {
    if (this->meleeTimer.getElapsedTime().asSeconds() >= this->meleeCooldown) {
        this->meleeTimer.restart();
    }
}

int Soldier::getState() const {
    return this->currentHP;
}

int Soldier::getLives() const {
    return this->lives;
}

int Soldier::getCurrentHP() const {
    return this->currentHP;
}

void Soldier::respawn() {
    this->currentHP = 3;
    this->health = this->maxHealth;
    this->position.x = 100.f;
    this->position.y = 300.f;
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->onGround = false;
}

void Soldier::setTransformationState(TransformationState* newState) {
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
        this->transformState = nullptr;
    }

    this->transformState = newState;

    if (this->transformState != nullptr) {
        this->transformState->applyEffects(this);
    }
}

TransformationState* Soldier::getTransformationState() const {
    return this->transformState;
}

void Soldier::handleJump() {
    if (this->onGround) {
        this->velocityY = -20.f;  
        this->onGround = false;
    }
}

void Soldier::applyGravity() {
    if (!this->onGround) {
        this->velocityY += 0.8f;
        if (this->velocityY > 20.f) {
            this->velocityY = 20.f;
        }
    }
}

void Soldier::handleCollision(Level* lvl) {
    if (lvl == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float playerLeft = this->position.x;
    
    
    
    float playerRight = this->position.x + 34.f * scaleX;
    float playerTop = this->position.y;
    float playerBottom = this->position.y + 40.f * scaleY;

    int cellSize = lvl->getCellSize();
    this->onGround = false;

    int startCol = static_cast<int>(playerLeft) / cellSize - 1;
    int endCol = static_cast<int>(playerRight) / cellSize + 1;
    int startRow = static_cast<int>(playerTop) / cellSize - 1;
    int endRow = static_cast<int>(playerBottom) / cellSize + 1;

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) continue;

            float blockLeft = static_cast<float>(col * cellSize);
            float blockRight = blockLeft + static_cast<float>(cellSize);
            float blockTop = static_cast<float>(row * cellSize);
            float blockBottom = blockTop + static_cast<float>(cellSize);

            if (playerRight > blockLeft && playerLeft < blockRight &&
                playerBottom > blockTop && playerTop < blockBottom) {

                float overlapLeft = playerRight - blockLeft;
                float overlapRight = blockRight - playerLeft;
                float overlapTop = playerBottom - blockTop;
                float overlapBottom = blockBottom - playerTop;

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

                playerLeft = this->position.x;
                playerRight = this->position.x + 34.f * scaleX;
                playerTop = this->position.y;
                playerBottom = this->position.y + 40.f * scaleY;
            }
        }
    }

    if (!this->onGround) {
        float probeY = playerBottom + 1.0f;
        int probeRow = static_cast<int>(probeY) / cellSize;
        int probeStartCol = static_cast<int>(playerLeft + 2) / cellSize;
        int probeEndCol = static_cast<int>(playerRight - 2) / cellSize;
        for (int col = probeStartCol; col <= probeEndCol; ++col) {
            if (lvl->isSolid(probeRow, col)) {
                this->onGround = true;
                break;
            }
        }
    }

    if (this->position.x < 0.f) {
        this->position.x = 0.f;
        this->velocityX = 0.f;
    }
    float maxPlayerX = (float)(lvl->getWidth()) * (float)(lvl->getCellSize())
        - 36.f * scaleX;
    if (this->position.x > maxPlayerX) {
        this->position.x = maxPlayerX;
        this->velocityX = 0.f;
    }
}

void Soldier::applyMovement(float& scroll) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = this->sprite.getScale().y;
    if (this->direction == DIR_LEFT) {
        this->sprite.setScale(-scaleX, scaleY);
    }
    else {
        this->sprite.setScale(scaleX, scaleY);
    }
}

void Soldier::handleStateTimers() {
    if (this->transformState != nullptr) {
        float dt = 0.016f;
        this->transformState->update(this, dt);

        if (this->transformState->isExpired()) {
            this->transformState->onExpiry(this);
        }
    }
}

void Soldier::setDirectionAndVelocity(int dir) {
    this->direction = dir;
    float accel = 0.5f;
    if (dir == DIR_LEFT) {
        this->velocityX -= accel;
        if (this->velocityX < -this->maxVelocity) this->velocityX = -this->maxVelocity;
    }
    else {
        this->velocityX += accel;
        if (this->velocityX > this->maxVelocity) this->velocityX = this->maxVelocity;
    }
}

void Soldier::decelerate() {
    if (this->onGround) {
        if (this->velocityX > 0.f) {
            this->velocityX -= 0.5f;
            if (this->velocityX < 0.f) this->velocityX = 0.f;
        }
        else if (this->velocityX < 0.f) {
            this->velocityX += 0.5f;
            if (this->velocityX > 0.f) this->velocityX = 0.f;
        }
    }
}

void Soldier::resolveBlockCollisions(DamagableEntity** blocks, int count) {
    if (blocks == nullptr || count == 0) return;

    IntRect pb = this->getBoundingBox();
    float playerLeft = static_cast<float>(pb.left);
    float playerRight = static_cast<float>(pb.left + pb.width);
    float playerTop = static_cast<float>(pb.top);
    float playerBottom = static_cast<float>(pb.top + pb.height);

    for (int i = 0; i < count; i++) {
        if (blocks[i] == nullptr) continue;
        if (!blocks[i]->isAlive() || !blocks[i]->getStatus()) continue;

        IntRect b = blocks[i]->getBoundingBox();

        float blockLeft = static_cast<float>(b.left);
        float blockRight = static_cast<float>(b.left + b.width);
        float blockTop = static_cast<float>(b.top);
        float blockBottom = static_cast<float>(b.top + b.height);

        if (playerRight <= blockLeft || playerLeft >= blockRight ||
            playerBottom <= blockTop || playerTop >= blockBottom) continue;

        float overlapLeft = playerRight - blockLeft;
        float overlapRight = blockRight - playerLeft;
        float overlapTop = playerBottom - blockTop;
        float overlapBottom = blockBottom - playerTop;

        bool movingDown = (this->velocityY > 0.5f);
        bool movingUp = (this->velocityY < -0.5f);
        bool movingHoriz = (this->velocityX > 0.5f || this->velocityX < -0.5f);

        float minOverlap = overlapLeft;
        int   resolveDir = 1;

        if (overlapRight < minOverlap) { minOverlap = overlapRight;  resolveDir = 2; }
        if (overlapTop < minOverlap) { minOverlap = overlapTop;    resolveDir = 3; }
        if (overlapBottom < minOverlap) { minOverlap = overlapBottom; resolveDir = 4; }

        if (movingHoriz && !movingDown) {
            float sideOverlap = overlapLeft;
            int   sideDir = 1;
            if (overlapRight < sideOverlap) { sideOverlap = overlapRight; sideDir = 2; }
            if (sideOverlap < overlapTop * 2.f) {
                minOverlap = sideOverlap;
                resolveDir = sideDir;
            }
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
            if (this->velocityY < 0.f) this->velocityY = 0.f;
        }

        pb = this->getBoundingBox();
        playerLeft = static_cast<float>(pb.left);
        playerRight = static_cast<float>(pb.left + pb.width);
        playerTop = static_cast<float>(pb.top);
        playerBottom = static_cast<float>(pb.top + pb.height);
    }
}