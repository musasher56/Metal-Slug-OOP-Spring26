#include "Collectible.h"
Collectible::Collectible(TextureManager* texMgr, AudioManager* audMgr): Entity(texMgr, audMgr), boundingBox(0, 0, 32, 32){}

Collectible::~Collectible() {}

void Collectible::update(float scroll, void* lvl) {
    (void)scroll;
    (void)lvl;
}

void Collectible::draw(RenderWindow& window, float scrollX, float scrollY) {
    this->sprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    this->animation.applyToSprite(this->sprite);
    window.draw(this->sprite);
}

IntRect Collectible::getBoundingBox() const {
    return this->boundingBox;
}

InteractableObject::InteractableObject(TextureManager* texMgr, AudioManager* audMgr): Entity(texMgr, audMgr), boundingBox(0, 0, 32, 48), interactRadius(50.f){}

InteractableObject::~InteractableObject() {}

bool InteractableObject::isInRange(sf::Vector2f pos) const {
    float dx = pos.x - this->position.x;
    float dy = pos.y - this->position.y;
    return (dx * dx + dy * dy) <= (this->interactRadius * this->interactRadius);
}

void InteractableObject::update(float scroll, void* lvl) {
    (void)scroll;
    (void)lvl;
}

void InteractableObject::draw(RenderWindow& window, float scrollX, float scrollY) {
    this->sprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    this->animation.applyToSprite(this->sprite);
    window.draw(this->sprite);
}

IntRect InteractableObject::getBoundingBox() const {
    return this->boundingBox;
}
Food::Food(TextureManager* texMgr, AudioManager* audMgr, bool turkey): Collectible(texMgr, audMgr),isTurkey(turkey), saturation(turkey ? 3 : 2){}

Food::~Food() {}

void Food::onPickup(CharacterManager* cm) {
    if (cm == nullptr) return;
    this->status = false;  
}

SupplyCrate::SupplyCrate(TextureManager* texMgr, AudioManager* audMgr): Collectible(texMgr, audMgr),containedWeapon(WEAPON_HMG),containedAmmo(50),handGrenades(2),fireBombGrenades(1){}

SupplyCrate::~SupplyCrate() {}

void SupplyCrate::onPickup(CharacterManager* cm) {
    if (cm == nullptr) return;
    this->status = false;  
}

void SupplyCrate::generateContents() {
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
    
    this->handGrenades = 2 + (std::rand() % 3);  
    this->fireBombGrenades = std::rand() % 2;    
}

POWPrisoner::POWPrisoner(TextureManager* texMgr, AudioManager* audMgr): InteractableObject(texMgr, audMgr),freed(false){}

POWPrisoner::~POWPrisoner() {}

void POWPrisoner::onInteract(CharacterManager* cm) {
    if (this->freed || cm == nullptr) return;
    this->free(cm);
}

void POWPrisoner::free(CharacterManager* cm) {
    this->freed = true;
    this->status = false;  
}
