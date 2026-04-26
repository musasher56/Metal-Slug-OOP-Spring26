#include "Player.h"

Player::Player()
    : x(380.f)
    , y(300.f)
    , velX(0.f)
    , velY(0.f)
    , onGround(false)
    , facingRight(true)
    , maxSpeed(5.f)
    , acceleration(0.5f)
    , width(30)
    , height(50)
    , scale_x(0.2f)
    , scale_y(0.2f)
    , gravity(1.f, 20.f, -20.f)   // WHY: same values as tumblepop
{
    this->playerTex.loadFromFile("resources/Sprites/Character.png");
    this->playerSprite.setTexture(this->playerTex);
    this->playerSprite.setScale(this->scale_x, this->scale_y);
}

Player::~Player() {}

void Player::moveLeft() {
    this->velX -= this->acceleration;
    if (this->velX < -this->maxSpeed) this->velX = -this->maxSpeed;
    this->facingRight = false;
}

void Player::moveRight() {
    this->velX += this->acceleration;
    if (this->velX > this->maxSpeed) this->velX = this->maxSpeed;
    this->facingRight = true;
}

void Player::stop() {
    // WHY: only decelerate when on ground — in the air we keep momentum
    // This is what makes jump+move forward feel correct
    if (this->onGround) {
        if (this->velX > 0.f) {
            this->velX -= this->acceleration;
            if (this->velX < 0.f) this->velX = 0.f;
        } else if (this->velX < 0.f) {
            this->velX += this->acceleration;
            if (this->velX > 0.f) this->velX = 0.f;
        }
    }
    // WHY: when airborne, velX carries naturally — no deceleration
    // player just drifts forward, which feels like real jump physics
}

void Player::jump() {
    this->gravity.jump(this->velY, this->onGround);
}

void Player::update(char** lvl, int lvlH, int lvlW, int cellSize) {
    // WHY: gravity handles vertical movement and collision
    this->gravity.update(this->x, this->y, this->velY, this->onGround,
                         lvl, lvlH, lvlW, cellSize,
                         this->width, this->height);

    // WHY: horizontal movement applied separately so gravity doesn't affect it
    this->x += this->velX;

    // WHY: flip sprite based on direction
    if (this->facingRight)
        this->playerSprite.setScale( this->scale_x, this->scale_y);
    else
        this->playerSprite.setScale(-this->scale_x, this->scale_y);
}

void Player::draw(sf::RenderWindow& window) {
    this->playerSprite.setPosition(this->x, this->y);
    window.draw(this->playerSprite);
}