#include "TransformationState.h"
#include "Soldier.h"

TransformationState::TransformationState(int transformType)
    : type(transformType)
    , duration(10.0f)  // 10 seconds for both UNDEAD and MUMMY
{}

TransformationState::~TransformationState() {}

int TransformationState::getType() const {
    return this->type;
}

bool TransformationState::isExpired() const {
    return this->durationTimer.getElapsedTime().asSeconds() >= this->duration;
}

// ========== NormalState Implementation ==========

NormalState* NormalState::instance = nullptr;

NormalState::NormalState()
    : TransformationState(TRANSFORM_NONE)
{}

NormalState* NormalState::getInstance() {
    // WHY: Lazy initialization of singleton
    if (NormalState::instance == nullptr) {
        NormalState::instance = new NormalState();
    }
    return NormalState::instance;
}

void NormalState::applyEffects(Soldier* s) {
    // WHY: No-op - NormalState has no effects
    // Just resets soldier to normal behavior
    (void)s;  // Suppress unused warning
}

void NormalState::onExpiry(Soldier* s) {
    // WHY: No-op - NormalState doesn't expire
    // If somehow expired, stay in NormalState
    (void)s;  // Suppress unused warning
}

void NormalState::update(Soldier* s, float dt) {
    // WHY: No-op - NormalState has no timers or effects to update
    (void)s;
    (void)dt;
}

// ========== UndeadState Implementation ==========

UndeadState::UndeadState()
    : TransformationState(TRANSFORM_UNDEAD)
    , speedMultiplier(0.50f)  // 50% speed penalty
{}

void UndeadState::applyEffects(Soldier* s) {
    // WHY: Apply 50% walk-speed penalty to soldier
    if (s != nullptr) {
        float newBase = s->getBaseMaxVelocity() * this->speedMultiplier;
        s->setBaseMaxVelocity(newBase);
        s->setMaxVelocity(newBase);
    }
}

void UndeadState::onExpiry(Soldier* s) {
    // WHY: Restore normal state when expired
    // Sets transformation back to NormalState singleton
    if (s != nullptr) {
        s->setTransformationState(NormalState::getInstance());
    }
}

void UndeadState::update(Soldier* s, float dt) {
    // WHY: No additional update logic needed
    // Duration timer in base class handles expiry
    (void)s;
    (void)dt;
}

// ========== MummyState Implementation ==========

MummyState::MummyState()
    : TransformationState(TRANSFORM_MUMMY)
    , weaponLocked(true)  // Lock weapons, only knife allowed
{}

void MummyState::applyEffects(Soldier* s) {
    // WHY: Lock weapon usage - player can only use melee/knife
    // Actual weapon locking handled in PlayerSoldier::shoot()
    (void)s;  // Effect is checked via isWeaponLocked()
}

void MummyState::onExpiry(Soldier* s) {
    // WHY: Restore normal state when expired
    // Sets transformation back to NormalState singleton
    if (s != nullptr) {
        s->setTransformationState(NormalState::getInstance());
    }
}

void MummyState::update(Soldier* s, float dt) {
    // WHY: No additional update logic needed
    // Duration timer in base class handles expiry
    (void)s;
    (void)dt;
}

bool MummyState::isWeaponLocked() const {
    return this->weaponLocked;
}
