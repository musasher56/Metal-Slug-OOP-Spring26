#pragma once
#include "Entity.h"

// Forward declarations
class CharacterManager;

// WHY: Collectible is the abstract base class for all pickup items
// Extends Entity with bounding box for collision detection
class Collectible : public Entity {
protected:
    IntRect boundingBox;

public:
    Collectible(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Collectible();

    virtual void onPickup(CharacterManager* cm) = 0;
    
    virtual void update(float scroll, void* lvl);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    
    IntRect getBoundingBox() const;
};

// ========== InteractableObject : Entity ==========
// Requires player interaction (key press) rather than walk-over pickup
class InteractableObject : public Entity {
protected:
    IntRect boundingBox;
    float interactRadius;

public:
    InteractableObject(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~InteractableObject();

    virtual void onInteract(CharacterManager* cm) = 0;
    
    bool isInRange(sf::Vector2f pos) const;
    virtual void update(float scroll, void* lvl);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    
    IntRect getBoundingBox() const;
};

// ========== Food : Collectible ==========
// isTurkey: +3 saturation  fruit: +2 saturation
class Food : public Collectible {
private:
    bool isTurkey;
    int saturation;

public:
    Food(TextureManager* texMgr, AudioManager* audMgr, bool turkey);
    virtual ~Food();

    virtual void onPickup(CharacterManager* cm);
};

// ========== SupplyCrate : Collectible ==========
// Contains random weapon and ammo
class SupplyCrate : public Collectible {
private:
    int containedWeapon;  // WeaponType
    int containedAmmo;
    int handGrenades;
    int fireBombGrenades;

public:
    SupplyCrate(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~SupplyCrate();

    virtual void onPickup(CharacterManager* cm);
    void generateContents();  // 90% HMG/Flame/Rocket  10% Laser
};

// ========== POWPrisoner : InteractableObject ==========
// Requires key-press to free (was walk-over - corrected per spec)
class POWPrisoner : public InteractableObject {
private:
    bool freed;

public:
    POWPrisoner(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~POWPrisoner();

    virtual void onInteract(CharacterManager* cm);
    void free(CharacterManager* cm);  // Spawns SupplyCrate
};
