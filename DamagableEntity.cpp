#include "DamagableEntity.h"

DamagableEntity::DamagableEntity(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , health(3)
    , maxHealth(3)
    , boundingBox(0, 0, 32, 64)
    , scoreValue(0)
{
}

DamagableEntity::~DamagableEntity() {}

void DamagableEntity::takeDamage(int amount) {
    if (amount < 0) return;
    this->health -= amount;
    if (this->health < 0) this->health = 0;

    if (this->health <= 0) {
        this->onDeath();
    }
}

void DamagableEntity::takeDamageFrom(int amount, int bulletDir) {
    
    
    (void)bulletDir;
    this->takeDamage(amount);
}

bool DamagableEntity::isAlive() const {
    return this->health > 0;
}

IntRect DamagableEntity::getBoundingBox() const {
    return IntRect(
        static_cast<int>(this->position.x) + this->boundingBox.left,
        static_cast<int>(this->position.y) + this->boundingBox.top,
        this->boundingBox.width,
        this->boundingBox.height
    );
}

int DamagableEntity::getScoreValue() const {
    return this->scoreValue;
}