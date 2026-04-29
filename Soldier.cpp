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
    , currentHP(3)  // HEALTHY
    , saturation(100)
    , meleeDamage(1.f)
    , meleeCooldown(0.5f)
    , transformState(nullptr)
{}

Soldier::~Soldier() {
    // WHY: FIX #4 - Soldier does not own NormalState (singleton), but owns other states
    // NormalState has type TRANSFORM_NONE, so we only delete if type != TRANSFORM_NONE
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
    }
    this->transformState = nullptr;
}

void Soldier::update(float scroll, Level* lvl) {
    this->handleStateTimers();
    this->applyGravity();
    this->handleCollision(lvl);
    this->applyMovement(scroll);
}

void Soldier::draw(RenderWindow& window, float scroll) {
    this->animation.update();                          // advance frame
    this->animation.applyToSprite(this->sprite);       // apply to sprite
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    window.draw(this->sprite);
}

void Soldier::takeDamage(int amount) {
    if (amount < 0) return;
    
    // Check if transformation provides immunity or reduction
    if (this->transformState != nullptr) {
        // Undead state could have different damage handling
    }
    
    this->currentHP -= amount;
    if (this->currentHP < 0) this->currentHP = 0;
    
    // Update health state enum
    if (this->currentHP == 0) {
        this->onDeath();
    } else if (this->currentHP == 1) {
        // CRITICAL state
    } else if (this->currentHP == 2) {
        // INJURED state
    }
    
    DamagableEntity::takeDamage(amount);
}

void Soldier::meleeAttack() {
    if (this->meleeTimer.getElapsedTime().asSeconds() >= this->meleeCooldown) {
        // Perform melee attack logic here
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
    // WHY: FIX #4 - Delete previous state if it is NOT NormalState (singleton)
    // NormalState is a static singleton and must never be deleted
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
        this->velocityY = -12.f;  // Jump strength
        this->onGround = false;
    }
}

void Soldier::applyGravity() {
    if (!this->onGround) {
        this->velocityY += 0.8f;  // Gravity acceleration
        if (this->velocityY > 20.f) {
            this->velocityY = 20.f;  // Terminal velocity
        }
    }
}

void Soldier::handleCollision(Level* lvl) {
    // ROOT CAUSE 3 FIX: Null-guard handleCollision against null level
    if (lvl == nullptr) return;  // no collision without a level — safe to skip
    
    // Get player bounding box - use abs() on scale to handle flipped sprites
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float playerLeft = this->position.x;
    float playerRight = this->position.x + 32.f * scaleX;
    float playerTop = this->position.y;
    float playerBottom = this->position.y + 40.f * scaleY;
    
    int cellSize = lvl->getCellSize();
    this->onGround = false;
    
    // Check cells around the player
    int startCol = static_cast<int>(playerLeft) / cellSize - 1;
    int endCol = static_cast<int>(playerRight) / cellSize + 1;
    int startRow = static_cast<int>(playerTop) / cellSize - 1;
    int endRow = static_cast<int>(playerBottom) / cellSize + 1;
    
    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) continue;
            
            // Solid block bounds
            float blockLeft = static_cast<float>(col * cellSize);
            float blockRight = blockLeft + static_cast<float>(cellSize);
            float blockTop = static_cast<float>(row * cellSize);
            float blockBottom = blockTop + static_cast<float>(cellSize);
            
            // Check for overlap
            if (playerRight > blockLeft && playerLeft < blockRight &&
                playerBottom > blockTop && playerTop < blockBottom) {
                
                // Collision detected - resolve based on velocity
                float overlapLeft = playerRight - blockLeft;
                float overlapRight = blockRight - playerLeft;
                float overlapTop = playerBottom - blockTop;
                float overlapBottom = blockBottom - playerTop;
                
                // Find minimum overlap
                float minOverlap = overlapLeft;
                int resolveDir = 0; // 1=left, 2=right, 3=top, 4=bottom
                
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
                
                // Resolve collision
                if (resolveDir == 1) {
                    this->position.x -= minOverlap;
                    this->velocityX = 0.f;
                } else if (resolveDir == 2) {
                    this->position.x += minOverlap;
                    this->velocityX = 0.f;
                } else if (resolveDir == 3) {
                    this->position.y -= minOverlap;
                    this->velocityY = 0.f;
                    this->onGround = true;
                } else if (resolveDir == 4) {
                    this->position.y += minOverlap;
                    this->velocityY = 0.f;
                }
                
                // Update player bounds after resolution - use abs() on scale
                playerLeft = this->position.x;
                playerRight = this->position.x + 32.f * scaleX;
                playerTop = this->position.y;
                playerBottom = this->position.y + 40.f * scaleY;
            }
        }
    }
}

void Soldier::applyMovement(float& scroll) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
    
    // Apply direction to sprite - preserve existing scale magnitude, only flip X sign
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = this->sprite.getScale().y;
    if (this->direction == DIR_LEFT) {
        this->sprite.setScale(-scaleX, scaleY);
    } else {
        this->sprite.setScale(scaleX, scaleY);
    }
}

void Soldier::handleStateTimers() {
    if (this->transformState != nullptr) {
        float dt = 0.016f;  // Approximate frame time
        this->transformState->update(this, dt);
        
        if (this->transformState->isExpired()) {
            this->transformState->onExpiry(this);
        }
    }
}

// ROOT CAUSE 4 FIX: Helper methods for movement control from CharacterManager
void Soldier::setDirectionAndVelocity(int dir) {
    this->direction = dir;
    float accel = 0.5f;
    if (dir == DIR_LEFT) {
        this->velocityX -= accel;
        if (this->velocityX < -this->maxVelocity) this->velocityX = -this->maxVelocity;
    } else {
        this->velocityX += accel;
        if (this->velocityX > this->maxVelocity) this->velocityX = this->maxVelocity;
    }
}

void Soldier::decelerate() {
    if (this->onGround) {
        if (this->velocityX > 0.f) {
            this->velocityX -= 0.5f;
            if (this->velocityX < 0.f) this->velocityX = 0.f;
        } else if (this->velocityX < 0.f) {
            this->velocityX += 0.5f;
            if (this->velocityX > 0.f) this->velocityX = 0.f;
        }
    }
}
