#include "MummyState.h"
#include "Soldier.h"
#include "NormalState.h"

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
