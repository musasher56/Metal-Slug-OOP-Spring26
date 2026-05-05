#include "Level.h"
#include "Constants.h"

Level::Level()
    : lvl(nullptr)
    , height(40)
    , width(240)
    , cell_size(48)
{
    // WHY: Allocate 2D grid for level blocks using manual memory management.
    // Height 40 rows × 48px = 1920px tall world — taller than SCREEN_H (720px)
    // so vertical scrolling has range to follow the player up mountains.
    this->lvl = new char* [this->height];
    for (int i = 0; i < this->height; i++) {
        this->lvl[i] = new char[this->width]();  // Zero-initialize
    }

    // WHY: Ground is the very last row so the floor sits at the world bottom.
    // Row 39 * 48 = 1872px — well below screen, camera scrolls down to follow.
    int groundRow = this->height - 1;  // row 39
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

void Level::Draw(sf::RenderWindow& window, float scrollX, float scrollY) {
    // WHY: Platform is invisible — collision works via grid data, no visual tiles drawn
    // The 'g' tiles still exist in the grid for physics/collision but are not rendered
    (void)scrollX;
    (void)scrollY;
}