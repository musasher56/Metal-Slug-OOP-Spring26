#pragma once
#include "TransformationState.h"

// WHY: MummyState forces knife-only combat, expires after 10s
// Set by MummyWarrior collision - player can only use melee
class MummyState : public TransformationState {
private:
    bool weaponLocked;  // true - disables shoot()

public:
    MummyState();
    
    virtual void applyEffects(Soldier* s) override;
    virtual void onExpiry(Soldier* s) override;
    virtual void update(Soldier* s, float dt) override;
    
    bool isWeaponLocked() const;
};
