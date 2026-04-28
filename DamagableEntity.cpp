#include "DamagableEntity.h"

DamagableEntity::DamagableEntity(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , health(3)
    , maxHealth(3)
    , boundingBox(0, 0, 32, 64)
    , scoreValue(0)
{}

DamagableEntity::~DamagableEntity() {}

void DamagableEntity::takeDamage(int amount) {
    if (amount < 0) return;
    this->health -= amount;
    if (this->health < 0) this->health = 0;
    
    if (this->health <= 0) {
        this->onDeath();
    }
}

bool DamagableEntity::isAlive() const {
    return this->health > 0;
}

IntRect DamagableEntity::getBoundingBox() const {
    return this->boundingBox;
}

int DamagableEntity::getScoreValue() const {
    return this->scoreValue;
}
