#include "Level.h"
#include "Constants.h"

Level::Level()
    : lvl(nullptr)
    , height(16)
    , width(240)
    , cell_size(48)
{
    // WHY: Allocate 2D grid for level blocks using manual memory management
    this->lvl = new char* [this->height];
    for (int i = 0; i < this->height; i++) {
        this->lvl[i] = new char[this->width]();  // Zero-initialize
    }

    // WHY: Ground is the very last row so the floor sits at the screen bottom.
    // Row 14 was the old value — 14 * 48 = 672px — which landed mid-screen and
    // created the "invisible floating platform" bug.
    // Row 15 * 48 = 720px which is at the bottom of a 768px screen.
    int groundRow = this->height - 1;  // row 15
    for (int j = 0; j < this->width; j++) {
        this->lvl[groundRow][j] = 'g';
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
    // WHY: Platform is invisible — collision works via grid data, no visual tiles drawn
    // The 'g' tiles still exist in the grid for physics/collision but are not rendered
}