#include "Grenade.h"



Grenade::Grenade(int radius, int dmg)
    : position(0.f, 0.f)
    , blastRadius(radius)
    , damage(dmg)
    , velocity(0.f, 0.f)
    , active(false)
{}

Grenade::~Grenade() {}

void Grenade::update() {
    if (!this->active) return;
    
    
    this->velocity.y += 0.5f;
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
}

void Grenade::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->active) return;
    
    
    
    (void)window;
    (void)scrollX;
    (void)scrollY;
}

bool Grenade::isActive() const {
    return this->active;
}

sf::Vector2f Grenade::getPosition() const {
    return this->position;
}

int Grenade::getBlastRadius() const {
    return this->blastRadius;
}

int Grenade::getDamage() const {
    return this->damage;
}



HandGrenade::HandGrenade()
    : Grenade(3, 5)  
{}

HandGrenade::~HandGrenade() {}

void HandGrenade::launch(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active = true;
    
    
    sf::Vector2f direction = target - origin;
    direction.x /= 20.f;  
    direction.y = -10.f;   
    
    this->velocity = direction;
}

void HandGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    
    
    
    if (em != nullptr) {
        
    }
}



FireBombGrenade::FireBombGrenade()
    : Grenade(3, 2)  
    , poolActive(false)
    , poolRadius(3)
    , poolLifetime(10.f)
{}

FireBombGrenade::~FireBombGrenade() {}

void FireBombGrenade::launch(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active = true;
    
    
    sf::Vector2f direction = target - origin;
    direction.x /= 20.f;
    direction.y = -10.f;
    
    this->velocity = direction;
}

void FireBombGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    
    
    this->spawnFirePool();
    
    
    if (em != nullptr) {
        
    }
}

void FireBombGrenade::spawnFirePool() {
    this->poolActive = true;
    this->poolDuration.restart();
    
    
    
}
