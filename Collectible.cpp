#include "Collectible.h"

// ========== Collectible Implementation ==========

Collectible::Collectible(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , boundingBox(0, 0, 32, 32)
{}

Collectible::~Collectible() {}

void Collectible::update(float scroll, void* lvl) {
    // WHY: Collectibles don't move or update - just sit there waiting for pickup
    (void)scroll;
    (void)lvl;
}

void Collectible::draw(RenderWindow& window, float scroll) {
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    this->animation.applyToSprite(this->sprite);
    window.draw(this->sprite);
}

IntRect Collectible::getBoundingBox() const {
    return this->boundingBox;
}

// ========== InteractableObject Implementation ==========

InteractableObject::InteractableObject(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , boundingBox(0, 0, 32, 48)
    , interactRadius(50.f)
{}

InteractableObject::~InteractableObject() {}

bool InteractableObject::isInRange(sf::Vector2f pos) const {
    float dx = pos.x - this->position.x;
    float dy = pos.y - this->position.y;
    return (dx * dx + dy * dy) <= (this->interactRadius * this->interactRadius);
}

void InteractableObject::update(float scroll, void* lvl) {
    // WHY: Interactables don't move - just wait for player interaction
    (void)scroll;
    (void)lvl;
}

void InteractableObject::draw(RenderWindow& window, float scroll) {
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    this->animation.applyToSprite(this->sprite);
    window.draw(this->sprite);
}

IntRect InteractableObject::getBoundingBox() const {
    return this->boundingBox;
}

// ========== Food Implementation ==========

Food::Food(TextureManager* texMgr, AudioManager* audMgr, bool turkey)
    : Collectible(texMgr, audMgr)
    , isTurkey(turkey)
    , saturation(turkey ? 3 : 2)  // Turkey: +3, Fruit: +2
{}

Food::~Food() {}

void Food::onPickup(CharacterManager* cm) {
    if (cm == nullptr) return;
    
    // WHY: Restore saturation to player character
    // Actual implementation needs CharacterManager to modify current character
    // cm->getCurrentCharacter()->saturation += this->saturation;
    
    this->status = false;  // Deactivate collectible
}

// ========== SupplyCrate Implementation ==========

SupplyCrate::SupplyCrate(TextureManager* texMgr, AudioManager* audMgr)
    : Collectible(texMgr, audMgr)
    , containedWeapon(WEAPON_HMG)
    , containedAmmo(50)
    , handGrenades(2)
    , fireBombGrenades(1)
{}

SupplyCrate::~SupplyCrate() {}

void SupplyCrate::onPickup(CharacterManager* cm) {
    if (cm == nullptr) return;
    
    // WHY: Give contents to player
    // Actual implementation needs CharacterManager to modify current character's inventory
    // cm->getCurrentCharacter()->addAmmo(this->containedAmmo);
    // cm->getCurrentCharacter()->addGrenades(this->handGrenades);
    
    this->status = false;  // Deactivate collectible
}

void SupplyCrate::generateContents() {
    // WHY: 90% HMG/Flame/Rocket, 10% Laser
    int rand = std::rand() % 100;
    if (rand < 30) {
        this->containedWeapon = WEAPON_HMG;
        this->containedAmmo = 100;
    } else if (rand < 60) {
        this->containedWeapon = WEAPON_FLAME_SHOT;
        this->containedAmmo = 50;
    } else if (rand < 90) {
        this->containedWeapon = WEAPON_ROCKET_LAUNCHER;
        this->containedAmmo = 10;
    } else {
        this->containedWeapon = WEAPON_LASER_GUN;
        this->containedAmmo = 20;
    }
    
    this->handGrenades = 2 + (std::rand() % 3);  // 2-4 grenades
    this->fireBombGrenades = std::rand() % 2;    // 0-1 fire bombs
}

// ========== POWPrisoner Implementation ==========

POWPrisoner::POWPrisoner(TextureManager* texMgr, AudioManager* audMgr)
    : InteractableObject(texMgr, audMgr)
    , freed(false)
{}

POWPrisoner::~POWPrisoner() {}

void POWPrisoner::onInteract(CharacterManager* cm) {
    if (this->freed || cm == nullptr) return;
    
    this->free(cm);
}

void POWPrisoner::free(CharacterManager* cm) {
    this->freed = true;
    
    // WHY: Spawns SupplyCrate when freed
    // Actual implementation needs CollectibleManager to spawn crate at this position
    // cm->spawnSupplyCrate(this->position);
    
    this->status = false;  // Deactivate prisoner
}
