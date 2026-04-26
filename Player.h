#pragma once
#include <SFML/Graphics.hpp>
#include "Gravity.h"

class Player {
public:
    Player();
    ~Player();

    void moveLeft();
    void moveRight();
    void stop();         // WHY: renamed Stop -> stop (consistent lowercase)
    void jump();
    void update(char** lvl, int lvlH, int lvlW, int cellSize);
    void draw(sf::RenderWindow& window);

    float getX()       const { return this->x; }
    float getY()       const { return this->y; }
    bool  isOnGround() const { return this->onGround; }

private:
    float x;
    float y;
    float velX;
    float velY;
    bool  onGround;
    bool  facingRight;

    float maxSpeed;
    float acceleration;

    int   width;    // WHY: collision box width used by Gravity
    int   height;   // WHY: collision box height used by Gravity

    float scale_x;
    float scale_y;

    Gravity gravity; // WHY: composition — Player HAS-A Gravity, enemies can too

    sf::Texture playerTex;
    sf::Sprite  playerSprite;
};