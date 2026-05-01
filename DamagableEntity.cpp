#include "DamagableEntity.h"

DamagableEntity::DamagableEntity(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , health(3)
    , maxHealth(3)
    , boundingBox(0, 0, 32, 64)  // LOCAL offset and size — NOT world position
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

// ------------------------------------------------------------------
// getBoundingBox — returns WORLD-SPACE bounding box
// ------------------------------------------------------------------
// WHY add position here?
//   The original implementation returned (0, 0, 32, 64) regardless of
//   where the entity was — a static local box.  That means every entity
//   appeared to be at the world origin for collision purposes.
//
//   The fix: boundingBox stores the LOCAL offset and size.
//   getBoundingBox() adds the entity's world position so the returned
//   IntRect can be directly compared against a projectile's world-space box.
//
// WHY is boundingBox still stored as a local offset?
//   Subclasses (Soldier, Vehicle, Enemy) set boundingBox in their
//   constructors to define the shape of their hitbox relative to their
//   origin sprite point.  If we stored it in world space it would need
//   to be updated every frame as the entity moves — that's fragile.
//   Local offset + position addition at query time is the cleaner pattern.
//
// Your friend's destructible blocks need this same fix:
//   their Block class should also store a local IntRect for the hitbox
//   and add world position in getBoundingBox().
// ------------------------------------------------------------------
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