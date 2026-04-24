#pragma once
#include <SFML/Graphics.hpp>

class Player {
private:
    float player_x ;
    float player_y ;

    float max_speed ;
    float velocityX;
    float acceleration;

    float scale_x ;
    float scale_y ;

    int raw_img_x ;
    int raw_img_y ;

    int Pheight;
    int Pwidth;
    bool facingRight;

    sf::Texture playerTex;
    sf::Sprite playerSprite;
public:
    Player();

    void moveLeft();
    void moveRight();
    void Stop();
    void Update();
    void Draw(sf::RenderWindow& window);
    float getX() const { return player_x; }
    float getY() const { return player_y; }
    ~Player();
};