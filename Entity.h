#pragma once
#include "Constants.h"
#include "Vector2d.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "Animation.h"

// Forward declaration to avoid circular dependency
class Level;

// WHY: Entity is the root abstract class for all game objects
// Provides common properties: position, sprite, animation, status
class Entity {
public:
    sf::Vector2f position;
    Sprite sprite;
    Animation animation;
    bool status;  // active/inactive

protected:
    TextureManager* textureManager;
    AudioManager* audioManager;

public:
    Entity(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Entity();

    sf::Vector2f getPosition() const;
    bool getStatus() const;
    void deactivate();

    virtual void updateAnimation();
    virtual void update(float scroll, Level* lvl) = 0;
    virtual void draw(RenderWindow& window, float scrollX, float scrollY) = 0;
};
