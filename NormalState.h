#pragma once
#include "TransformationState.h"

// WHY: NormalState is the default transformation state - a static singleton
// Never deleted by setTransformationState() - FIX #4
class NormalState : public TransformationState {
private:
    static NormalState* instance;
    
    NormalState();  // Private constructor for singleton

public:
    static NormalState* getInstance();
    
    virtual void applyEffects(Soldier* s) override;
    virtual void onExpiry(Soldier* s) override;
    virtual void update(Soldier* s, float dt) override;
};
