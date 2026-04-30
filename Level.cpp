#include "Level.h"
#include "Constants.h"

Level::Level()
    : lvl(nullptr)
    , height(16)
    , width(150)
    , cell_size(48)
{
    // WHY: Allocate 2D grid for level blocks using manual memory management
    this->lvl = new char*[this->height];
    for (int i = 0; i < this->height; i++) {
        this->lvl[i] = new char[this->width]();  // Zero-initialize
    }

    // WHY: Place a simple ground platform for testing collision
    for (int j = 0; j < this->width; j++) {
        this->lvl[14][j] = 'g';  // row 14 * 48 = 672px — slightly lower
    }

    // WHY: Load block texture; fallback handled by caller if missing
    this->wallTex1.loadFromFile("resources/Sprites/blocks/grass_block_side.png");
    this->wallSprite1.setTexture(this->wallTex1);

    // Scale wall sprite to match new cell_size regardless of source texture dimensions
    float texW = static_cast<float>(this->wallTex1.getSize().x);
    float texH = static_cast<float>(this->wallTex1.getSize().y);
    if (texW > 0.f && texH > 0.f) {
        this->wallSprite1.setScale(
            static_cast<float>(this->cell_size) / texW,
            static_cast<float>(this->cell_size) / texH
        );
    }
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

void Level::Draw(sf::RenderWindow& window, float scroll) {
    // WHY: Only draw tiles that are visible on screen (culling for performance)
    int startCol = static_cast<int>(scroll / this->cell_size) - 1;
    int endCol = static_cast<int>((scroll + SCREEN_W) / this->cell_size) + 1;
    if (startCol < 0) startCol = 0;
    if (endCol >= this->width) endCol = this->width - 1;

    // WHY: Iterate grid and draw only solid blocks ('g' = grass)
    for (int i = 0; i < this->height; i++) {
        for (int j = startCol; j <= endCol; j++) {
            if (this->lvl[i][j] == 'g') {
                this->wallSprite1.setPosition(
                    static_cast<float>(j * this->cell_size) - scroll,
                    static_cast<float>(i * this->cell_size)
                );
                window.draw(this->wallSprite1);
            }
        }
    }
}