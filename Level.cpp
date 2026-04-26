#include "Level.h"

Level::Level()
    : lvl(nullptr)
    , height(14)
    , width(110)
    , cell_size(64)
{
    // WHY: Allocate 2D grid for level blocks using manual memory management
    this->lvl = new char*[this->height];
    for (int i = 0; i < this->height; i++) {
        this->lvl[i] = new char[this->width]();  // Zero-initialize
    }

    // WHY: Place a simple ground platform for testing collision
    for (int j = 0; j < this->width; j++) {
        this->lvl[10][j] = 'g';  // row 10 * 64 = 640px — visible on screen
    }

    // WHY: Load block texture; fallback handled by caller if missing
    this->wallTex1.loadFromFile("resources/Sprites/blocks/grass_block_side.png");
    this->wallSprite1.setTexture(this->wallTex1);
}

Level::~Level() {
    // WHY: Clean up dynamically allocated 2D array to prevent memory leaks
    for (int i = 0; i < this->height; i++) {
        delete[] this->lvl[i];
        this->lvl[i] = nullptr;
    }
    delete[] this->lvl;
    this->lvl = nullptr;
}

void Level::Draw(sf::RenderWindow& window) {
    // WHY: Iterate grid and draw only solid blocks ('g' = grass)
    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            if (this->lvl[i][j] == 'g') {
                this->wallSprite1.setPosition(
                    static_cast<float>(j * this->cell_size),
                    static_cast<float>(i * this->cell_size)
                );
                window.draw(this->wallSprite1);
            }
        }
    }
}