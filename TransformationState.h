#pragma once
#include "Constants.h"

// Forward declaration to avoid circular dependency
class Soldier;

// WHY: TransformationState implements State pattern for Soldier transformations
// States: Normal, Undead, Mummy - each with different effects and expiry behavior
class TransformationState {
protected:
    int type;  // TransformationType constant
    Clock durationTimer;
    float duration;  // 10s for both UNDEAD and MUMMY

public:
    TransformationState(int transformType);
    virtual ~TransformationState();
    
    virtual void applyEffects(Soldier* s) = 0;
    virtual void onExpiry(Soldier* s) = 0;
    virtual void update(Soldier* s, float dt) = 0;
    int getType() const;
    bool isExpired() const;
};
