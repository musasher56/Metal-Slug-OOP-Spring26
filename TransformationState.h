#pragma once
#include "Constants.h"


class Soldier;



class TransformationState {
protected:
    int type;  
    Clock durationTimer;
    float duration;  

public:
    TransformationState(int transformType);
    virtual ~TransformationState();
    
    virtual void applyEffects(Soldier* s) = 0;
    virtual void onExpiry(Soldier* s) = 0;
    virtual void update(Soldier* s, float dt) = 0;
    int getType() const;
    bool isExpired() const;
};



class NormalState : public TransformationState {
private:
    static NormalState* instance;
    
    NormalState();  

public:
    static NormalState* getInstance();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
};



class UndeadState : public TransformationState {
private:
    float speedMultiplier;  

public:
    UndeadState();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
};



class MummyState : public TransformationState {
private:
    bool weaponLocked;  

public:
    MummyState();
    
    virtual void applyEffects(Soldier* s);
    virtual void onExpiry(Soldier* s);
    virtual void update(Soldier* s, float dt);
    
    bool isWeaponLocked() const;
};
