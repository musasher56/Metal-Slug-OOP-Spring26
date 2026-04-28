#pragma once
#include "TransformationState.h"

// WHY: UndeadState applies 50% walk-speed penalty, expires after 10s
// Set by Zombie collision - player becomes undead temporarily
class UndeadState : public TransformationState {
private:
    float speedMultiplier;  // 0.50 = 50% speed

public:
    UndeadState();
    
    virtual void applyEffects(Soldier* s) override;
    virtual void onExpiry(Soldier* s) override;
    virtual void update(Soldier* s, float dt) override;
};
