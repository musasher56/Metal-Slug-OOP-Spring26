#pragma once
#include "Entity.h"

// Forward declarations
class EnemyManager;
class CharacterManager;

// WHY: Grenade is the abstract base class for all grenades
// Provides common properties: position, blastRadius, damage, velocity
class Grenade {
protected:
    sf::Vector2f position;
    int blastRadius;
    int damage;
    sf::Vector2f velocity;
    bool active;

public:
    Grenade(int radius, int dmg);
    virtual ~Grenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target) = 0;
    virtual void onImpact(EnemyManager* em) = 0;
    
    void update();
    void draw(RenderWindow& window, float scroll);
    bool isActive() const;
    
    sf::Vector2f getPosition() const;
    int getBlastRadius() const;
    int getDamage() const;
};

// ========== HandGrenade : Grenade ==========
// BALLISTIC arc 5 HP in 3-block radius
class HandGrenade : public Grenade {
public:
    HandGrenade();
    virtual ~HandGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);
};

// ========== FireBombGrenade : Grenade ==========
// Eri's default grenade - creates persistent fire pool
class FireBombGrenade : public Grenade {
private:
    Clock poolDuration;
    bool poolActive;
    int poolRadius;  // 3 blocks
    float poolLifetime;  // 10 seconds

public:
    FireBombGrenade();
    virtual ~FireBombGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);
    
    void spawnFirePool();
};
