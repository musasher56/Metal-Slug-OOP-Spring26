#pragma once
#include <SFML/Graphics.hpp>

class NoiseProfile;  // Forward declaration — Level.cpp includes the full definition

// =============================================================================
// Level
//
// Represents a single game level as a 2D grid of block characters:
//   '\0' = empty air
//   'g'  = solid ground (destructible)
//   'i'  = indestructible bedrock (bottom row only)
//   'w'  = water (aquatic biome fill)
//
// TWO CONSTRUCTION MODES:
//
//   Level()                      — Survival mode.
//     Creates a flat-ground level (240 x 40 grid). Bottom row is solid.
//     Enemies are placed by LevelManager at predefined positions.
//
//   Level(NoiseProfile* profile) — Campaign mode.
//     Creates a noise-generated level (CAMPAIGN_WIDTH x CAMPAIGN_HEIGHT).
//     Samples PerlinNoise::fractal() per column using profile parameters.
//     Supports infinite horizontal scrolling via advanceWorld().
//     Level does NOT own the profile pointer — caller manages lifetime.
//
// INFINITE TERRAIN (campaign mode):
//   The grid is a sliding window over an infinite noise field.
//   worldOffsetX tracks which world-space column maps to grid column 0.
//   advanceWorld(steps, profile) shifts the buffer left and generates
//   fresh columns on the right edge. Because Perlin noise is deterministic
//   (same x → same output), scrolling left regenerates identical terrain.
//
// BIOME SYSTEM:
//   heightMap[col] stores the normalized noise output (0.0–1.0) for each
//   column. getBiomeAt(col) thresholds this into BIOME_AERIAL / BIOME_PLAINS
//   / BIOME_AQUATIC. EnemyManager and spawners query this to place enemies
//   in the correct terrain zone.
//
// MEMORY NOTE (P3):
//   Level owns: lvl (char**), heightMap (float*).
//   Level does NOT own: NoiseProfile* passed to constructor/advanceWorld.
//   The Rule of Three is satisfied: destructor cleans both arrays.
//   Copy constructor and copy assignment are deleted (no shallow-copy risk).
// =============================================================================

// Grid dimensions
static const int SURVIVAL_WIDTH   = 240;
static const int SURVIVAL_HEIGHT  = 40;
static const int CAMPAIGN_WIDTH   = 420;   // Sliding window: wider than screen
static const int CAMPAIGN_HEIGHT  = 15;
static const int CELL_SIZE        = 48;

// Biome height thresholds (from PerlinNoise::fractal output in [0, 1])
static const float BIOME_AERIAL_THRESHOLD  = 0.65f;  // above this → aerial
static const float BIOME_AQUATIC_THRESHOLD = 0.35f;  // below this → aquatic
// between the two → plains


class Level {
private:

    char** lvl;         // 2D grid [row][col]
    int    height;      // Number of rows
    int    width;       // Number of columns (may be larger than screen for campaign)
    int    cellSize;    // Pixel size of each block

    float* heightMap;   // Normalized noise height per column [0.0, 1.0]
                        // nullptr in survival mode (flat terrain has no height variation)

    int    worldOffsetX; // World-space X of grid column 0 (campaign mode only)
    bool   isCampaign;   // Distinguishes construction mode for Draw and getBiomeAt

    // -------------------------------------------------------------------------
    // generateColumn
    // Fills one grid column from fractal noise output.
    //
    // col     - grid column index [0, width)
    // worldX  - world-space X position for noise sampling (deterministic)
    // profile - noise parameters; not stored, only read here
    //
    // Block placement rules:
    //   - Bottom row (height-1):  always 'i' (indestructible bedrock)
    //   - Surface row down to height-2: 'g' (solid ground)
    //   - Above surface in aquatic zone: 'w' (water up to sea level row)
    //   - Everything above ground: '\0' (air)
    // -------------------------------------------------------------------------
    void generateColumn(int col, int worldX, NoiseProfile* profile);

    // -------------------------------------------------------------------------
    // clearColumn: Zeroes a single column (sets all cells to '\0').
    // Called before generateColumn to avoid stale block data.
    // -------------------------------------------------------------------------
    void clearColumn(int col);

public:

    // Survival mode constructor — flat level, no noise
    Level();

    // Campaign mode constructor — noise-generated terrain
    // profile: borrowed pointer, Level does NOT delete it
    explicit Level(NoiseProfile* profile);

    // Destructor — cleans lvl (char**) and heightMap (float*)
    ~Level();

    // Delete copy operations — Level owns raw arrays, shallow copy would double-free
    Level(const Level&)            = delete;
    Level& operator=(const Level&) = delete;

    // -------------------------------------------------------------------------
    // Draw: Renders visible blocks as colored rectangles.
    // Only processes columns and rows within the camera viewport to avoid
    // iterating the full (potentially wide) campaign grid every frame.
    //
    // Block colors:
    //   'i' = dark grey (bedrock)
    //   'w' = semi-transparent blue (water)
    //   'g' = color varies by biome: grey(aerial) / brown(plains) / dark green(aquatic)
    //         Top-surface blocks get a lighter "grass" strip for readability.
    // -------------------------------------------------------------------------
    void Draw(sf::RenderWindow& window, float scrollX, float scrollY);

    // -------------------------------------------------------------------------
    // retreatWorld: Slides the terrain buffer LEFT by 'steps' columns.
    // Shifts existing data right, generates 'steps' new columns on the left.
    // Mirror of advanceWorld — enables infinite scrolling in BOTH directions.
    // Same noise function: retreating over previously seen terrain regenerates
    // identical blocks (deterministic from worldOffsetX + column index).
    // -------------------------------------------------------------------------
    void retreatWorld(int steps, NoiseProfile* profile);

    // -------------------------------------------------------------------------
    // advanceWorld: Slides the terrain buffer right by 'steps' columns.
    //
    // Copies existing grid data left by 'steps' positions, then calls
    // generateColumn() for the 'steps' new columns appearing on the right.
    // heightMap is shifted the same way.
    // worldOffsetX advances by 'steps'.
    //
    // Only valid in campaign mode (isCampaign == true).
    // steps should be small (typically 1–20 per call) for smooth streaming.
    // O(width * height) shift cost — ~21,000 ops for 420x50, very fast.
    // -------------------------------------------------------------------------
    void advanceWorld(int steps, NoiseProfile* profile);

    // -------------------------------------------------------------------------
    // getBiomeAt: Returns BIOME_AERIAL, BIOME_PLAINS, or BIOME_AQUATIC
    // for the given grid column based on stored heightMap value.
    // In survival mode (heightMap == nullptr), always returns BIOME_PLAINS.
    // -------------------------------------------------------------------------
    int getBiomeAt(int col) const;

    // --- Getters ---
    bool   isCampaignMode()  const { return isCampaign;   }  // for PlayState campaign checks
    int    getHeight()       const { return height;       }
    int    getWidth()        const { return width;        }
    int    getCellSize()     const { return cellSize;     }
    int    getWorldOffset()  const { return worldOffsetX; }
    char** getLvl()          const { return lvl;          }

    // --- Block type queries ---

    // Solid = 'g' (ground) OR 'i' (indestructible).
    // Both block movement. Important: indestructible must NOT be exempt.
    bool isSolid(int row, int col) const {
        if (row < 0 || row >= height || col < 0 || col >= width) return false;
        char c = lvl[row][col];
        return c == 'g' || c == 'i';
    }

    bool isWater(int row, int col) const {
        if (row < 0 || row >= height || col < 0 || col >= width) return false;
        return lvl[row][col] == 'w';
    }

    bool isIndestructible(int row, int col) const {
        if (row < 0 || row >= height || col < 0 || col >= width) return false;
        return lvl[row][col] == 'i';
    }

    // -------------------------------------------------------------------------
    // setSolid: Modifies a block (for blast destruction).
    // Silently ignores attempts to destroy indestructible bedrock — the spec
    // states bottom-row blocks must be indestructible under ANY circumstances.
    // -------------------------------------------------------------------------
    void setSolid(int row, int col, bool solid) {
        if (row < 0 || row >= height || col < 0 || col >= width) return;
        if (lvl[row][col] == 'i') return;  // Cannot destroy bedrock — spec rule
        lvl[row][col] = solid ? 'g' : '\0';
    }
};