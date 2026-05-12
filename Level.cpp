#include "Level.h"
#include "PerlinNoise.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>

static const float SEA_LEVEL_FRACTION = 0.60f;

static const float GRASS_STRIP_HEIGHT = 5.0f;

Level::Level()
    : lvl(nullptr)
    , height(SURVIVAL_HEIGHT)
    , width(SURVIVAL_WIDTH)
    , cellSize(CELL_SIZE)
    , heightMap(nullptr)
    , worldOffsetX(0)
    , isCampaign(false)
    , dirtTexture(nullptr)
    , dirtReady(false)
{
    lvl = new char* [height];
    for (int r = 0; r < height; r++) {
        lvl[r] = new char[width](); 
    }

    for (int c = 0; c < width; c++) {
        lvl[height - 1][c] = 'i';
    }

    for (int c = 0; c < width; c++) {
        lvl[height - 2][c] = 'g';
    }
}

Level::Level(NoiseProfile* profile)
    : lvl(nullptr)
    , height(CAMPAIGN_HEIGHT)
    , width(CAMPAIGN_WIDTH)
    , cellSize(CELL_SIZE)
    , heightMap(nullptr)
    , worldOffsetX(-30)
    , isCampaign(true)
    , dirtTexture(nullptr)
    , dirtReady(false)
{
    lvl = new char* [height];
    for (int r = 0; r < height; r++) {
        lvl[r] = new char[width]();
    }

    heightMap = new float[width]();

    if (profile != nullptr) {
        for (int c = 0; c < width; c++) {
            generateColumn(c, worldOffsetX + c, profile);
        }
    }
}

Level::~Level() {
    if (lvl != nullptr) {
        for (int r = 0; r < height; r++) {
            delete[] lvl[r];
            lvl[r] = nullptr;
        }
        delete[] lvl;
        lvl = nullptr;
    }

    if (heightMap != nullptr) {
        delete[] heightMap;
        heightMap = nullptr;
    }
}

void Level::setDirtTexture(sf::Texture* tex) {
    this->dirtTexture = tex;
    if (tex != nullptr) {
        this->dirtSprite.setTexture(*tex);
        sf::Vector2u sz = tex->getSize();
        if (sz.x > 0 && sz.y > 0) {
            float sx = (float)cellSize / (float)sz.x;
            float sy = (float)cellSize / (float)sz.y;
            this->dirtSprite.setScale(sx, sy);
        }
        this->dirtReady = true;
    }
    else {
        this->dirtReady = false;
    }
}


void Level::clearColumn(int col) {
    for (int r = 0; r < height; r++) {
        lvl[r][col] = '\0';
    }
}


void Level::generateColumn(int col, int worldX, NoiseProfile* profile) {
    clearColumn(col);

    float h = PerlinNoise::fractal(
        static_cast<float>(worldX),
        profile->getAmplitude(),
        profile->getFrequency(),
        profile->getPersistence(),
        profile->getHarmonics(),
        profile->getSeed()
    );

    heightMap[col] = h;

    int surfaceRow = static_cast<int>((1.0f - h) * static_cast<float>(height - 2));
    if (surfaceRow < 1)          surfaceRow = 1;
    if (surfaceRow > height - 2) surfaceRow = height - 2;

    lvl[height - 1][col] = 'i';

    for (int r = surfaceRow; r < height - 1; r++) {
        lvl[r][col] = 'g';
    }

    if (h < BIOME_AQUATIC_THRESHOLD) {
        int seaLevelRow = static_cast<int>(SEA_LEVEL_FRACTION * static_cast<float>(height));
        if (seaLevelRow > height - 2) seaLevelRow = height - 2;

        for (int r = seaLevelRow; r < surfaceRow; r++) {
            if (lvl[r][col] == '\0') {
                lvl[r][col] = 'w';
            }
        }
    }
}

void Level::advanceWorld(int steps, NoiseProfile* profile) {
    if (!isCampaign || profile == nullptr) return;
    if (steps <= 0 || steps >= width)     return;

    int remaining = width - steps;

    for (int c = 0; c < remaining; c++) {
        heightMap[c] = heightMap[c + steps];
    }

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < remaining; c++) {
            lvl[r][c] = lvl[r][c + steps];
        }
    }

    int newWorldBase = worldOffsetX + width;
    for (int i = 0; i < steps; i++) {
        generateColumn(remaining + i, newWorldBase + i, profile);
    }

    worldOffsetX += steps;
}

void Level::retreatWorld(int steps, NoiseProfile* profile) {
    if (!isCampaign || profile == nullptr) return;
    if (steps <= 0 || steps >= width)     return;

    int remaining = width - steps;

    for (int c = remaining - 1; c >= 0; c--) {
        heightMap[c + steps] = heightMap[c];
    }

    for (int r = 0; r < height; r++) {
        for (int c = remaining - 1; c >= 0; c--) {
            lvl[r][c + steps] = lvl[r][c];
        }
    }

    worldOffsetX -= steps;
    for (int i = 0; i < steps; i++) {
        generateColumn(i, worldOffsetX + i, profile);
    }
}

int Level::getBiomeAt(int col) const {
    if (heightMap == nullptr) return BIOME_PLAINS;
    if (col < 0 || col >= width) return BIOME_PLAINS;

    float h = heightMap[col];

    if (h > BIOME_AERIAL_THRESHOLD)  return BIOME_AERIAL;
    if (h < BIOME_AQUATIC_THRESHOLD) return BIOME_AQUATIC;
    return BIOME_PLAINS;
}


int Level::getSurfaceRow(int col) const {
    if (col < 0 || col >= width) return height - 1;

    for (int r = 0; r < height; r++) {
        char c = lvl[r][col];
        if (c == 'g' || c == 'i') {
            return r;
        }
    }

    return height - 1;
}

void Level::Draw(sf::RenderWindow& window, float scrollX, float scrollY) {
 
    if (!isCampaign)
        return;

    int colStart = static_cast<int>(scrollX) / cellSize - worldOffsetX - 1;
    int colEnd = colStart + (SCREEN_W / cellSize) + 3;
    int rowStart = static_cast<int>(scrollY) / cellSize - 1;
    int rowEnd = rowStart + (SCREEN_H / cellSize) + 3;

    if (colStart < 0)      colStart = 0;
    if (colEnd > width)  colEnd = width;
    if (rowStart < 0)      rowStart = 0;
    if (rowEnd > height) rowEnd = height;

    sf::RectangleShape waterBlock(sf::Vector2f(
        static_cast<float>(cellSize),
        static_cast<float>(cellSize)
    ));

    sf::RectangleShape grassStrip(sf::Vector2f(
        static_cast<float>(cellSize),
        GRASS_STRIP_HEIGHT
    ));

    for (int r = rowStart; r < rowEnd; r++) {
        for (int c = colStart; c < colEnd; c++) {
            char cell = lvl[r][c];

            if (cell == '\0') continue;

            float screenX = static_cast<float>((worldOffsetX + c) * cellSize) - scrollX;
            float screenY = static_cast<float>(r * cellSize) - scrollY;

            if (cell == 'w') {
          
                waterBlock.setFillColor(sf::Color(20, 90, 200, 160));
                waterBlock.setPosition(screenX, screenY);
                window.draw(waterBlock);

            }
            else if (cell == 'i') {
         
                if (dirtReady) {
                    dirtSprite.setColor(sf::Color(80, 80, 90)); 
                    dirtSprite.setPosition(screenX, screenY);
                    window.draw(dirtSprite);
                }

            }
            else {
                bool topSurface = (r > 0 && lvl[r - 1][c] == '\0');

                if (dirtReady) {
                    int biome = getBiomeAt(c);

                    if (biome == BIOME_AERIAL) {
                     
                        dirtSprite.setColor(sf::Color(180, 175, 180));
                    }
                    else if (biome == BIOME_AQUATIC) {
                     
                        dirtSprite.setColor(sf::Color(140, 120, 100));
                    }
                    else {
                        dirtSprite.setColor(sf::Color::White);
                    }

                    dirtSprite.setPosition(screenX, screenY);
                    window.draw(dirtSprite);
                }

                if (topSurface) {
                    int biome = getBiomeAt(c);
                    if (biome == BIOME_AERIAL) {
                        grassStrip.setFillColor(sf::Color(160, 160, 170));
                    }
                    else if (biome == BIOME_AQUATIC) {
                        grassStrip.setFillColor(sf::Color(45, 120, 45));
                    }
                    else {
                        grassStrip.setFillColor(sf::Color(60, 140, 30));
                    }
                    grassStrip.setPosition(screenX, screenY);
                    window.draw(grassStrip);
                }
            }
        }
    }
}
