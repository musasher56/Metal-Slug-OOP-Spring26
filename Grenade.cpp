#include "Grenade.h"
#include <cmath>

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

    
    
    if (this->position.y > 2000.f) {
        this->active = false;
    }
}

void Grenade::draw(RenderWindow& window, float scrollX, float scrollY) {
    
    if (!this->active) return;

    sf::CircleShape body(6.f);
    body.setFillColor(sf::Color(80, 100, 30));    
    body.setOutlineColor(sf::Color(50, 70, 20));
    body.setOutlineThickness(1.5f);
    body.setOrigin(6.f, 6.f);
    body.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(body);

    
    
    float rotAngle = std::atan2f(this->velocity.y, this->velocity.x);
    sf::CircleShape pin(2.f);
    pin.setFillColor(sf::Color(220, 220, 220));
    pin.setOrigin(2.f, 2.f);
    pin.setPosition(
        (this->position.x - scrollX) + std::cosf(rotAngle) * 4.f,
        (this->position.y - scrollY) + std::sinf(rotAngle) * 4.f
    );
    window.draw(pin);
}

bool         Grenade::isActive()      const {
    return this->active;       }
sf::Vector2f Grenade::getPosition()   const {
    
    return this->position;     }
int          Grenade::getBlastRadius()const {
    return this->blastRadius;  }
int          Grenade::getDamage()     const { 
    return this->damage;       }












HandGrenade::HandGrenade()
    : Grenade(3, 5)
{}

HandGrenade::~HandGrenade() {}

void HandGrenade::launch(sf::Vector2f origin, sf::Vector2f target) {
    this->position = origin;
    this->active   = true;

    sf::Vector2f dir = target - origin;
    
    
    this->velocity.x = dir.x / 20.f;
    this->velocity.y = -10.f;
}

void HandGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    
    
    
    
    (void)em;
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
    this->active   = true;

    sf::Vector2f dir = target - origin;
    this->velocity.x = dir.x / 20.f;
    this->velocity.y = -10.f;
}

void FireBombGrenade::draw(RenderWindow& window, float scrollX, float scrollY) {
    
    if (this->active) {
        
        sf::CircleShape body(7.f);
        body.setFillColor(sf::Color(220, 80, 10));
        body.setOutlineColor(sf::Color(180, 40, 0));
        body.setOutlineThickness(1.5f);
        body.setOrigin(7.f, 7.f);
        body.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(body);
    }

    
    if (this->poolActive && this->poolDuration.getElapsedTime().asSeconds() < this->poolLifetime) {
        
        
        const int   cellSize  = 32;
        const float poolPx    = static_cast<float>(this->poolRadius * cellSize * 2);

        sf::RectangleShape pool(sf::Vector2f(poolPx, 16.f));
        pool.setFillColor(sf::Color(255, 60, 0, 120));
        pool.setOutlineColor(sf::Color(255, 120, 0, 180));
        pool.setOutlineThickness(2.f);
        pool.setOrigin(poolPx * 0.5f, 8.f);
        pool.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(pool);
    } else if (this->poolActive) {
        
        this->poolActive = false;
    }
}

void FireBombGrenade::onImpact(EnemyManager* em) {
    this->active = false;
    this->spawnFirePool();
    (void)em;
}

void FireBombGrenade::spawnFirePool() {
    
    this->poolActive = true;
    this->poolDuration.restart();
    
    
}