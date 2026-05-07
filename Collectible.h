#pragma once
#include "Entity.h"


class CharacterManager;



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



class Food : public Collectible {
private:
    bool isTurkey;
    int saturation;

public:
    Food(TextureManager* texMgr, AudioManager* audMgr, bool turkey);
    virtual ~Food();

    virtual void onPickup(CharacterManager* cm);
};



class SupplyCrate : public Collectible {
private:
    int containedWeapon;  
    int containedAmmo;
    int handGrenades;
    int fireBombGrenades;

public:
    SupplyCrate(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~SupplyCrate();

    virtual void onPickup(CharacterManager* cm);
    void generateContents();  
};



class POWPrisoner : public InteractableObject {
private:
    bool freed;

public:
    POWPrisoner(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~POWPrisoner();

    virtual void onInteract(CharacterManager* cm);
    void free(CharacterManager* cm);  
};
