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
    // ROOT CAUSE 2 FIX: applyToSprite sets texture rect if animation is set
    this->animation.applyToSprite(this->sprite);
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
    
    // Placeholder - actual implementation needs Level pointer
    // This will be implemented in concrete classes or with proper Level include
    (void)lvl;  // Suppress unused warning for now
}

void Soldier::applyMovement(float& scroll) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
    
    // Apply direction to sprite
    if (this->direction == DIR_LEFT) {
        this->sprite.setScale(-1.f, 1.f);
    } else {
        this->sprite.setScale(1.f, 1.f);
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
