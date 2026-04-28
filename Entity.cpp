#include "Entity.h"

Entity::Entity(TextureManager* texMgr, AudioManager* audMgr)
    : position(0.f, 0.f)
    , status(true)
    , textureManager(texMgr)
    , audioManager(audMgr)
{}

Entity::~Entity() {
    // WHY: Entity does not own textureManager or audioManager (shared pointers)
    this->textureManager = nullptr;
    this->audioManager = nullptr;
}

sf::Vector2f Entity::getPosition() const {
    return this->position;
}

bool Entity::getStatus() const {
    return this->status;
}

void Entity::deactivate() {
    this->status = false;
}

void Entity::updateAnimation() {
    this->animation.update();
}
