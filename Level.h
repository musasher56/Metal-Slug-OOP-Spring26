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

    int    getHeight()   const { return this->height; }
    int    getWidth()    const { return this->width; }
    int    getCellSize() const { return this->cell_size; }
    char** getLvl()      const { return this->lvl; } // needed for gravity checks

    bool isSolid(int row, int col) const {
        if (row < 0 || row >= this->height || col < 0 || col >= this->width)
            return false;
        return this->lvl[row][col] == 'g';
    }


};