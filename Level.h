#pragma once
#include <SFML/Graphics.hpp>

class NoiseProfile;  // forward declaration — avoids circular include

// ─────────────────────────────────────────────────────────────────────────────
// Level — 2D grid for terrain collision and rendering
//
// Two modes:
//   1. Survival Mode (default constructor): fixed 40×240 grid, bottom 2 rows
//      are ground/bedrock. BlockManager handles all visual dirt sprites.
//      Level::Draw() does nothing — BlockManager draws instead.
//
//   2. Campaign Mode (NoiseProfile* constructor): procedural terrain generated
//      from Perlin noise via NoiseProfile. Level::Draw() renders dirt texture
//      blocks with viewport culling and grass strips on surface blocks.
//      BlockManager is NOT used for campaign terrain visuals.
//
// Grid cell types:
//   '\0' — air (empty)
//   'g'  — solid ground (walkable, destructible)
//   'i'  — indestructible bedrock
//   'w'  — water (pass-through, rendered semi-transparent blue)
// ─────────────────────────────────────────────────────────────────────────────

class Level {
private:
    char** lvl;          // 2D grid [height][width]
    int     height;       // number of rows
    int     width;        // number of columns
    int     cellSize;     // pixels per cell (48)
    float* heightMap;    // one float per column [0..1] — only in campaign mode
    int     worldOffsetX; // world X offset for column 0 — only in campaign mode
    bool    isCampaign;   // true = campaign mode (procedural), false = survival

    // Dirt texture for campaign terrain rendering (set via setDirtTexture)
    sf::Texture* dirtTexture;
    sf::Sprite   dirtSprite;
    bool         dirtReady;  // true once dirt texture is loaded and sprite is set up

    // Internal helpers for campaign mode
    void clearColumn(int col);
    void generateColumn(int col, int worldX, NoiseProfile* profile);

public:
    // Survival mode constructor — fixed 40×240 grid with ground rows
    Level();

    // Campaign mode constructor — procedural terrain from NoiseProfile
    Level(NoiseProfile* profile);

    ~Level();

    // Draw the level (campaign mode only — survival uses BlockManager)
    void Draw(sf::RenderWindow& window, float scrollX, float scrollY);

    // ── Grid accessors ──
    int     getHeight()    const { return this->height; }
    int     getWidth()     const { return this->width; }
    int     getCellSize()  const { return this->cellSize; }
    char** getLvl()       const { return this->lvl; }
    bool    getCampaign()  const { return this->isCampaign; }
    int     getWorldOffX() const { return this->worldOffsetX; }

    // ── Collision ──
    // 'g' and 'i' are solid; '\0' and 'w' are not
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

    // ── Dirt texture for campaign rendering ──
    // Call this after constructing a campaign Level, before the first Draw().
    // The texture must outlive this Level object (owned by TextureManager).
    void setDirtTexture(sf::Texture* tex);

    // ── Campaign mode: world scrolling ──
    void advanceWorld(int steps, NoiseProfile* profile);
    void retreatWorld(int steps, NoiseProfile* profile);

    // ── Campaign mode: biome query ──
    int getBiomeAt(int col) const;

    // ── Surface lookup ──
    int getSurfaceRow(int col) const;
};
