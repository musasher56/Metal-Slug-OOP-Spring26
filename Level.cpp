#include "Level.h"

Level::Level()
    : lvl(nullptr), height(14), width(110), cell_size(64)
{
    lvl = new char* [height];

    for (int i = 0; i < height; i += 1) {
        lvl[i] = new char[width] {'\0'};
    }

  
    lvl[11][5] = 'g';
    lvl[11][6] = 'g';
    lvl[11][7] = 'g';
    lvl[11][8] = 'g';

    wallTex1.loadFromFile("resources/Sprites/blocks/grass_block_side.png");
    wallSprite1.setTexture(wallTex1);
}

Level::~Level()
{
    
    for (int i = 0; i < height; i += 1) {
        delete[] lvl[i];
    }
    delete[] lvl;
}

void Level::Draw(sf::RenderWindow& window)
{
    for (int i = 0; i < height; i += 1) {
        for (int j = 0; j < width; j += 1) {
            if (lvl[i][j] == 'g') {
                wallSprite1.setPosition(j * cell_size, i * cell_size);
                window.draw(wallSprite1);
            }
        }
    }
}