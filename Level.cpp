#include "Level.h"
#include "Constants.h"

Level::Level()
    : lvl(nullptr)
    , height(40)
    , width(240)
    , cell_size(48)
{
    
    
    
    this->lvl = new char* [this->height];
    for (int i = 0; i < this->height; i++) {
        this->lvl[i] = new char[this->width]();  
    }

    
    
    int groundRow = this->height - 1;  
    for (int j = 0; j < this->width; j++) {
        this->lvl[groundRow][j] = 'g';
    }
}

Level::~Level() {
    
    for (int i = 0; i < this->height; i++) {
        delete[] this->lvl[i];
        this->lvl[i] = nullptr;
    }
    delete[] this->lvl;
    this->lvl = nullptr;
}

void Level::Draw(sf::RenderWindow& window, float scrollX, float scrollY) {
    
    
    (void)scrollX;
    (void)scrollY;
}