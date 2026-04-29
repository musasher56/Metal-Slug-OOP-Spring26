#include "NormalState.h"
#include "Soldier.h"

// WHY: Static singleton instance - shared across all Soldiers, never deleted
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
