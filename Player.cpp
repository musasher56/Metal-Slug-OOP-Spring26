#include "Player.h"

Player::Player()
    : player_x(380.f)
    , player_y(500.f)      // WHY: moved up from 610 so player isn't inside ground
    , max_speed(5.f)
    , velocityX(0.f)
    , acceleration(0.5f)
    , scale_x(0.2f)
    , scale_y(0.2f)
    , raw_img_x(593)
    , raw_img_y(470)
    , Pheight(94)
    , Pwidth(119)
    , facingRight(true)
{
    this->playerTex.loadFromFile("resources/Sprites/Character.png");
    this->playerSprite.setTexture(this->playerTex);
    this->playerSprite.setScale(this->scale_x, this->scale_y);
}

Player::~Player() {}

void Player::moveLeft() {
    this->velocityX -= this->acceleration;
    if (this->velocityX < -this->max_speed)
        this->velocityX = -this->max_speed;
    this->facingRight = false;
}

void Player::moveRight() {
    this->velocityX += this->acceleration;
    if (this->velocityX > this->max_speed)
        this->velocityX = this->max_speed;
    this->facingRight = true;
}

void Player::Stop() {
    // WHY: decelerate smoothly instead of snapping to 0
    if (this->velocityX > 0.f) {
        this->velocityX -= this->acceleration;
        if (this->velocityX < 0.f) this->velocityX = 0.f;
    } else if (this->velocityX < 0.f) {
        this->velocityX += this->acceleration;
        if (this->velocityX > 0.f) this->velocityX = 0.f;
    }
}

void Player::Update() {
    this->player_x += this->velocityX;

    // WHY: flip sprite horizontally based on direction
    if (this->facingRight)
        this->playerSprite.setScale(this->scale_x, this->scale_y);
    else
        this->playerSprite.setScale(-this->scale_x, this->scale_y);
}

void Player::Draw(sf::RenderWindow& window) {
    this->playerSprite.setPosition(this->player_x, this->player_y);
    window.draw(this->playerSprite);
}