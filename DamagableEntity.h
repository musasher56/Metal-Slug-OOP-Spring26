#pragma once
#include "Entity.h"

class DamagableEntity : public Entity {
protected:
    int health;
    int maxHealth;
    IntRect boundingBox;
    int scoreValue;

public:
    DamagableEntity(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~DamagableEntity();
    virtual void takeDamage(int amount);
    virtual void takeDamageFrom(int amount, int bulletDir);
    virtual void onDeath() = 0;
    bool isAlive() const;
    IntRect getBoundingBox() const;
    int getScoreValue() const;
    virtual void updateBoundingBox() = 0;
};