#include "UndeadState.h"
#include "Soldier.h"
#include "NormalState.h"

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
