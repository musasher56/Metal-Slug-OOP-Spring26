#pragma once
#include <SFML/Graphics.hpp>
#include "Level.h"

class Character {
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Sprite sprite;
    sf::Texture* texture;
    bool onGround;
    float gravity;
    float jumpForce;
    float moveSpeed;
    int direction;
    const Level* level;

public:
    Character();
    ~Character();
    Character(const Character& other);
    Character& operator=(const Character& other);

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void handleInput(const sf::Event& event);
    void setLevel(const Level* lvl);
    void setPosition(float x, float y);  // <-- ADDED
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;

private:
    void applyPhysics(float dt);
    void resolveCollision();
};