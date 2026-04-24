#pragma once
#include <SFML/Graphics.hpp>

class Level {
private:
    char** lvl;
    int    height;
    int    width;
    int    cell_size;

    sf::Texture wallTex1;
    sf::Sprite  wallSprite1;

public:
    Level();
    ~Level();

    void Draw(sf::RenderWindow& window);

    int getHeight()   const { return height; }
    int getWidth()    const { return width; }
    int getCellSize() const { return cell_size; }
};

