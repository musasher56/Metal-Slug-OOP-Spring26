#include "TransformationState.h"
#include "Soldier.h"

TransformationState::TransformationState(int transformType)
    : type(transformType)
    , duration(10.0f)  
{}

TransformationState::~TransformationState() {}

int TransformationState::getType() const {
    return this->type;
}

bool TransformationState::isExpired() const {
    return this->durationTimer.getElapsedTime().asSeconds() >= this->duration;
}



NormalState* NormalState::instance = nullptr;

NormalState::NormalState()
    : TransformationState(TRANSFORM_NONE)
{}

NormalState* NormalState::getInstance() {
    
    if (NormalState::instance == nullptr) {
        NormalState::instance = new NormalState();
    }
    return NormalState::instance;
}

void NormalState::applyEffects(Soldier* s) {
    
    
    (void)s;  
}

void NormalState::onExpiry(Soldier* s) {
    
    
    (void)s;  
}

void NormalState::update(Soldier* s, float dt) {
    
    (void)s;
    (void)dt;
}



UndeadState::UndeadState()
    : TransformationState(TRANSFORM_UNDEAD)
    , speedMultiplier(0.50f)  
{}

void UndeadState::applyEffects(Soldier* s) {
    
    if (s != nullptr) {
        float newBase = s->getBaseMaxVelocity() * this->speedMultiplier;
        s->setBaseMaxVelocity(newBase);
        s->setMaxVelocity(newBase);
    }
}

void UndeadState::onExpiry(Soldier* s) {
    
    
    if (s != nullptr) {
        s->setTransformationState(NormalState::getInstance());
    }
}

void UndeadState::update(Soldier* s, float dt) {
    
    
    (void)s;
    (void)dt;
}



MummyState::MummyState()
    : TransformationState(TRANSFORM_MUMMY)
    , weaponLocked(true)  
{}

void MummyState::applyEffects(Soldier* s) {
    
    
    (void)s;  
}

void MummyState::onExpiry(Soldier* s) {
    
    
    if (s != nullptr) {
        s->setTransformationState(NormalState::getInstance());
    }
}

void MummyState::update(Soldier* s, float dt) {
    
    
    (void)s;
    (void)dt;
}

bool MummyState::isWeaponLocked() const {
    return this->weaponLocked;
}
