#include "Level.h"
#include "NoiseProfile.h"
#include "PerlinNoise.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>

// =============================================================================
// Sea level row (as a fraction of level height).
// In aquatic biomes (low noise output), the terrain surface is below this row.
// Water blocks fill from the surface up to this sea level so the ocean has
// visible depth rather than just stopping at the top of the ground.
// =============================================================================
static const float SEA_LEVEL_FRACTION = 0.60f;

// Grass strip height in pixels drawn on top of surface ground blocks
static const float GRASS_STRIP_HEIGHT = 5.0f;


// =============================================================================
// Survival Mode Constructor
// =============================================================================
Level::Level()
    : lvl(nullptr)
    , height(SURVIVAL_HEIGHT)
    , width(SURVIVAL_WIDTH)
    , cellSize(CELL_SIZE)
    , heightMap(nullptr)
    , worldOffsetX(0)
    , isCampaign(false)
{
    // Allocate the grid using raw arrays — no STL vectors (hard constraint)
    lvl = new char*[height];
    for (int r = 0; r < height; r++) {
        lvl[r] = new char[width]();  // value-initialize to '\0' (empty air)
    }

    // Bottom row: indestructible bedrock as per spec
    for (int c = 0; c < width; c++) {
        lvl[height - 1][c] = 'i';
    }

    // Second-from-bottom row: solid walkable ground
    for (int c = 0; c < width; c++) {
        lvl[height - 2][c] = 'g';
    }

    // heightMap is nullptr in survival mode — getBiomeAt() returns PLAINS for all cols
}


// =============================================================================
// Campaign Mode Constructor
// =============================================================================
Level::Level(NoiseProfile* profile)
    : lvl(nullptr)
    , height(CAMPAIGN_HEIGHT)
    , width(CAMPAIGN_WIDTH)
    , cellSize(CELL_SIZE)
    , heightMap(nullptr)
    , worldOffsetX(-30)  // 30 columns left of world x=0 = 1440px of left-side terrain
    , isCampaign(true)
{
    // Allocate grid
    lvl = new char*[height];
    for (int r = 0; r < height; r++) {
        lvl[r] = new char[width]();
    }

    // Allocate heightMap — one float per column
    heightMap = new float[width]();

    // Generate all columns in the initial window.
    // worldX = 0 + col for the initial population.
    // As the player scrolls, advanceWorld() keeps this window current.
    if (profile != nullptr) {
        for (int c = 0; c < width; c++) {
            generateColumn(c, worldOffsetX + c, profile);  // worldX offset by -30
        }
    }
}


// =============================================================================
// Destructor
// =============================================================================
Level::~Level() {
    // Free each row array first, then the row-pointer array
    if (lvl != nullptr) {
        for (int r = 0; r < height; r++) {
            delete[] lvl[r];
            lvl[r] = nullptr;
        }
        delete[] lvl;
        lvl = nullptr;
    }

    // heightMap is a flat 1D array (not jagged) — single delete[]
    if (heightMap != nullptr) {
        delete[] heightMap;
        heightMap = nullptr;
    }

    // NOTE: Level does NOT delete the NoiseProfile pointer.
    // Ownership stays with whoever called Level(NoiseProfile*) — typically
    // LevelManager. Deleting it here would cause a double-free if LevelManager
    // also deletes it. This is aggregation, not composition.
}


// =============================================================================
// clearColumn: Zeroes one grid column before regenerating it.
// =============================================================================
void Level::clearColumn(int col) {
    for (int r = 0; r < height; r++) {
        lvl[r][col] = '\0';
    }
}


// =============================================================================
// generateColumn
// =============================================================================
void Level::generateColumn(int col, int worldX, NoiseProfile* profile) {
    // Clear existing data in this column first
    clearColumn(col);

    // Sample the fractal noise for this world position.
    // Result h is in [0.0, 1.0]: 1.0 = maximum height (aerial peak), 0.0 = seafloor.
    float h = PerlinNoise::fractal(
        static_cast<float>(worldX),
        profile->getAmplitude(),
        profile->getFrequency(),
        profile->getPersistence(),
        profile->getHarmonics(),
        profile->getSeed()
    );

    // Store for biome queries (getBiomeAt, enemy spawning)
    heightMap[col] = h;

    // Convert normalized height to a grid row for the terrain surface.
    // h=1.0 → surfaceRow near the top (row 1, leaving row 0 as sky)
    // h=0.0 → surfaceRow near the bottom (height-2, leaving bedrock row alone)
    // The -2 margin keeps us away from the indestructible bottom row.
    int surfaceRow = static_cast<int>((1.0f - h) * static_cast<float>(height - 2));
    if (surfaceRow < 1)          surfaceRow = 1;
    if (surfaceRow > height - 2) surfaceRow = height - 2;

    // Bottom row is always indestructible bedrock (spec requirement)
    lvl[height - 1][col] = 'i';

    // Fill solid ground from surfaceRow down to height-2
    for (int r = surfaceRow; r < height - 1; r++) {
        lvl[r][col] = 'g';
    }

    // Aquatic biome: fill water from surface up to sea level.
    // Sea level is a fixed row fraction — water always has the same "ceiling"
    // regardless of terrain depth, creating a consistent ocean surface.
    if (h < BIOME_AQUATIC_THRESHOLD) {
        int seaLevelRow = static_cast<int>(SEA_LEVEL_FRACTION * static_cast<float>(height));
        if (seaLevelRow > height - 2) seaLevelRow = height - 2;

        // Fill from sea level down to (but not including) the surface row.
        // If surfaceRow <= seaLevelRow, the terrain is underwater — water fills above it.
        for (int r = seaLevelRow; r < surfaceRow; r++) {
            if (lvl[r][col] == '\0') {  // Only overwrite air, not existing ground
                lvl[r][col] = 'w';
            }
        }
    }
}


// =============================================================================
// advanceWorld
// =============================================================================
void Level::advanceWorld(int steps, NoiseProfile* profile) {
    // Only valid in campaign mode and with a valid profile
    if (!isCampaign || profile == nullptr) return;
    if (steps <= 0 || steps >= width)     return;

    int remaining = width - steps;  // Columns that survive the shift

    // --- Shift heightMap left by 'steps' ---
    // memmove-style manual shift (raw array, no STL)
    for (int c = 0; c < remaining; c++) {
        heightMap[c] = heightMap[c + steps];
    }

    // --- Shift each row of lvl left by 'steps' ---
    // This is O(width * height). For 420*50 = 21,000 byte copies per advance
    // call — negligible CPU cost even at 60 FPS.
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < remaining; c++) {
            lvl[r][c] = lvl[r][c + steps];
        }
    }

    // --- Generate 'steps' new columns on the right edge ---
    // worldX = worldOffsetX + width is the world position of the first new column.
    // Since noise is deterministic, scrolling back left and calling advanceWorld
    // with negative steps (or a symmetric retreatWorld) would regenerate
    // identical terrain. For simplicity we only support rightward scrolling here.
    int newWorldBase = worldOffsetX + width;
    for (int i = 0; i < steps; i++) {
        generateColumn(remaining + i, newWorldBase + i, profile);
    }

    // Advance the world origin
    worldOffsetX += steps;
}


// =============================================================================
// retreatWorld
// =============================================================================
void Level::retreatWorld(int steps, NoiseProfile* profile) {
    // Mirror of advanceWorld — generates terrain to the LEFT as player scrolls back.
    if (!isCampaign || profile == nullptr) return;
    if (steps <= 0 || steps >= width)     return;

    int remaining = width - steps;

    // --- Shift heightMap RIGHT by 'steps' (open up 'steps' slots on the left) ---
    // Must iterate from right to left to avoid overwriting data before copying.
    for (int c = remaining - 1; c >= 0; c--) {
        heightMap[c + steps] = heightMap[c];
    }

    // --- Shift each lvl row RIGHT by 'steps' ---
    for (int r = 0; r < height; r++) {
        for (int c = remaining - 1; c >= 0; c--) {
            lvl[r][c + steps] = lvl[r][c];
        }
    }

    // --- Generate 'steps' new columns on the LEFT edge ---
    // First: update worldOffsetX so column 0 now maps to the new leftmost world position.
    // Then generate using the updated worldOffsetX so worldX = worldOffsetX + i is correct.
    worldOffsetX -= steps;
    for (int i = 0; i < steps; i++) {
        generateColumn(i, worldOffsetX + i, profile);
    }
}


// =============================================================================
// getBiomeAt
// =============================================================================
int Level::getBiomeAt(int col) const {
    // No heightMap in survival mode — treat everything as plains
    if (heightMap == nullptr) return BIOME_PLAINS;
    if (col < 0 || col >= width) return BIOME_PLAINS;

    float h = heightMap[col];

    if (h > BIOME_AERIAL_THRESHOLD)  return BIOME_AERIAL;
    if (h < BIOME_AQUATIC_THRESHOLD) return BIOME_AQUATIC;
    return BIOME_PLAINS;
}


// =============================================================================
// Draw
// Renders only the visible portion of the level (viewport culling).
// =============================================================================
void Level::Draw(sf::RenderWindow& window, float scrollX, float scrollY) {
    // Survival mode: Level::Draw does nothing.
    // BlockManager owns all visual sprites for survival terrain.
    // Drawing here too would render opaque colored rectangles over/behind
    // BlockManager's dirt sprites — causing the "glass block" double-render.
    if (!isCampaign) return;

    // --- Determine visible range ---
    // We expand by 1 tile on each side to prevent pop-in at the edges.
    // Column c maps to world x = (worldOffsetX + c) * cellSize.
    // Viewport starts at scrollX in world space. First visible grid column:
    //   (worldOffsetX + c) * cellSize >= scrollX - cellSize
    //   c >= scrollX/cellSize - worldOffsetX - 1
    int colStart = static_cast<int>(scrollX) / cellSize - worldOffsetX - 1;
    int colEnd   = colStart + (SCREEN_W / cellSize) + 3;
    int rowStart = static_cast<int>(scrollY) / cellSize - 1;
    int rowEnd   = rowStart + (SCREEN_H / cellSize) + 3;

    // Clamp to grid bounds
    if (colStart < 0)      colStart = 0;
    if (colEnd   > width)  colEnd   = width;
    if (rowStart < 0)      rowStart = 0;
    if (rowEnd   > height) rowEnd   = height;

    // Reusable rectangle shape — avoids constructing/destructing per block
    sf::RectangleShape block(sf::Vector2f(
        static_cast<float>(cellSize),
        static_cast<float>(cellSize)
    ));

    // Thin grass strip drawn on top surfaces (separate smaller rectangle)
    sf::RectangleShape grassStrip(sf::Vector2f(
        static_cast<float>(cellSize),
        GRASS_STRIP_HEIGHT
    ));

    for (int r = rowStart; r < rowEnd; r++) {
        for (int c = colStart; c < colEnd; c++) {
            char cell = lvl[r][c];

            // Air — nothing to draw, skip immediately
            if (cell == '\0') continue;

            // World x = (worldOffsetX + c) * cellSize; screen x = worldX - camera
            float screenX = static_cast<float>((worldOffsetX + c) * cellSize) - scrollX;
            float screenY = static_cast<float>(r * cellSize) - scrollY;

            // --- Choose block color based on type and biome ---
            sf::Color blockColor;
            sf::Color grassColor;
            bool drawGrass = false;

            if (cell == 'i') {
                // Indestructible bedrock: very dark grey with a faint blue tint
                blockColor = sf::Color(35, 35, 45);

            } else if (cell == 'w') {
                // Water: semi-transparent blue
                // Alpha 160 lets background (enemy shadows, etc.) show through slightly
                blockColor = sf::Color(20, 90, 200, 160);

            } else {
                // Solid ground ('g'): color varies by biome for visual clarity

                // Determine if this is a top-surface block (block above is air or water).
                // Top-surface blocks get a grass strip overlay.
                bool topSurface = (r > 0 && lvl[r - 1][c] == '\0');

                int biome = getBiomeAt(c);

                if (biome == BIOME_AERIAL) {
                    // Rocky grey mountain terrain
                    blockColor = sf::Color(115, 115, 125);
                    grassColor = sf::Color(160, 160, 170);  // Light grey "snow"
                } else if (biome == BIOME_AQUATIC) {
                    // Dark muddy earth below the waterline
                    blockColor = sf::Color(65, 88, 50);
                    grassColor = sf::Color(45, 120, 45);    // Darker aquatic moss
                } else {
                    // Plains: standard brown earth
                    blockColor = sf::Color(101, 67, 33);
                    grassColor = sf::Color(60, 140, 30);    // Green grass
                }

                drawGrass = topSurface;
            }

            // Draw main block
            block.setFillColor(blockColor);
            block.setPosition(screenX, screenY);
            window.draw(block);

            // Draw grass strip on top-surface ground blocks
            if (drawGrass) {
                grassStrip.setFillColor(grassColor);
                grassStrip.setPosition(screenX, screenY);  // Same top-left corner
                window.draw(grassStrip);
            }
        }
    }
}