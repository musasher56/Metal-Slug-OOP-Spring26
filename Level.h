#pragma once
#include <SFML/Graphics.hpp>

class NoiseProfile;  


class Level {
private:
    char** lvl;         
    int     height;       
    int     width;       
    int     cellSize;    
    float* heightMap;    
    int     worldOffsetX; 
    bool    isCampaign;   

    sf::Texture* dirtTexture;
    sf::Sprite   dirtSprite;
    bool         dirtReady;  

    void clearColumn(int col);
    void generateColumn(int col, int worldX, NoiseProfile* profile);

public:
    Level();

    Level(NoiseProfile* profile);

    ~Level();

    void Draw(sf::RenderWindow& window, float scrollX, float scrollY);

    int     getHeight()    const { 
        return this->height; }
    int     getWidth()     const {
        return this->width; }
    int     getCellSize()  const {
        return this->cellSize; }
    char** getLvl()       const {
        return this->lvl; }
    bool    getCampaign()  const {
        return this->isCampaign; }
    int     getWorldOffX() const {
        return this->worldOffsetX; }

    bool isSolid(int row, int col) const {
        if (row < 0 || row >= this->height || col < 0 || col >= this->width)
            return false;
        char c = this->lvl[row][col];
        return (c == 'g' || c == 'i');
    }

    void setSolid(int row, int col, bool solid) {
        if (row < 0 || row >= this->height || col < 0 || col >= this->width)
            return;
        this->lvl[row][col] = solid ? 'g' : '\0';
    }

    void setDirtTexture(sf::Texture* tex);

    void advanceWorld(int steps, NoiseProfile* profile);
    void retreatWorld(int steps, NoiseProfile* profile);

    int getBiomeAt(int col) const;

    int getSurfaceRow(int col) const;
};
