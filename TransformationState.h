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

// WHY: NormalState is the default transformation state - a static singleton
// Never deleted by setTransformationState() - FIX #4
class NormalState : public TransformationState {
private:
    static NormalState* instance;
    
    NormalState();  // Private constructor for singleton

public:
    static NormalState* getInstance();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
};

// WHY: UndeadState applies 50% walk-speed penalty, expires after 10s
// Set by Zombie collision - player becomes undead temporarily
class UndeadState : public TransformationState {
private:
    float speedMultiplier;  // 0.50 = 50% speed

public:
    UndeadState();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
};

// WHY: MummyState forces knife-only combat, expires after 10s
// Set by MummyWarrior collision - player can only use melee
class MummyState : public TransformationState {
private:
    bool weaponLocked;  // true - disables shoot()

public:
    MummyState();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
    
    bool isWeaponLocked() const;
};
