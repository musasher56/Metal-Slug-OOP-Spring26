#include "Player.h"
using namespace std;

Player::Player():player_x(380), player_y(610), max_speed(5), velocityX (0), acceleration(0.5f), scale_x(0.2), scale_y(0.2), raw_img_x(593), raw_img_y(470), Pheight(94), Pwidth(119){
    playerTex.loadFromFile("resources/Sprites/Character.png");
    playerSprite.setTexture(playerTex);
    playerSprite.setScale(scale_x, scale_y);
}

void Player :: moveLeft() {
    velocityX -= acceleration;
    if (velocityX < -max_speed) velocityX = -max_speed;
}

void Player :: moveRight() {
    velocityX += acceleration;
    if (velocityX > max_speed) velocityX = +max_speed;
}

void Player::Stop() {
    player_x = 0.0f;
}

void Player::Update() {
    player_x += velocityX;
    //Y baad men dalen gay gravity bana kr abhi nhi
}

void Player::Draw(sf::RenderWindow& window) {
    playerSprite.setPosition(player_x,player_y);
    window.draw(playerSprite);
}