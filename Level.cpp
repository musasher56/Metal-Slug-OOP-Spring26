#include "Level.h"
#include "Constants.h"

Level::Level()
    : lvl(nullptr)
    , height(16)
    , width(240)
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

    // WHY: Platform is invisible — no texture loading needed
    // Collision still works via isSolid() reading the grid
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
    // WHY: Platform is invisible — collision works via grid data, no visual tiles drawn
    // The 'g' tiles still exist in the grid for physics/collision but are not rendered
}