#pragma once
#include <SFML/Graphics.hpp>

class Level {
private:
    char** lvl;
    int height;
    int width;
    int cell_size;
    sf::Texture wallTex1;
    sf::Sprite wallSprite1;

public:
    Level();
    ~Level();
    void Draw(sf::RenderWindow& window);  // WHY: Render level blocks to window

    // WHY: Inline getters for test compatibility (direct access allowed)
    int getHeight() const { return this->height; }
    int getWidth() const { return this->width; }
    int getCellSize() const { return this->cell_size; }

    // WHY: Query if a grid cell is solid (for collision detection)
    bool isSolid(int row, int col) const {
        if (row < 0 || row >= this->height || col < 0 || col >= this->width) {
            return false;
        }
        return this->lvl[row][col] == 'g';
    }
};