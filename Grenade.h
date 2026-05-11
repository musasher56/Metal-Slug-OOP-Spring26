#pragma once
#include "Entity.h"


class EnemyManager;
class CharacterManager;


















class Grenade {
protected:
    sf::Vector2f position;
    int          blastRadius;
    int          damage;
    sf::Vector2f velocity;
    bool         active;

public:
    Grenade(int radius, int dmg);
    virtual ~Grenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target) = 0;
    virtual void onImpact(EnemyManager* em) = 0;

    void update();

    
    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    bool         isActive()      const;
    sf::Vector2f getPosition()   const;
    int          getBlastRadius()const;
    int          getDamage()     const;
};



class HandGrenade : public Grenade {
public:
    HandGrenade();
    virtual ~HandGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);
    
};



class FireBombGrenade : public Grenade {
private:
    Clock poolDuration;
    bool  poolActive;
    int   poolRadius;
    float poolLifetime;

public:
    FireBombGrenade();
    virtual ~FireBombGrenade();

    virtual void launch(sf::Vector2f origin, sf::Vector2f target);
    virtual void onImpact(EnemyManager* em);

    
    
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);

    void spawnFirePool();
};