#pragma once
#include "Constants.h"

// ─────────────────────────────────────────────────────────────────────────────
// Per-enemy spawn entry — one enemy per slot, placed at (x,y) in world coords
// ─────────────────────────────────────────────────────────────────────────────
struct EnemySpawnEntry {
    int   type;     // ENEMY_REBEL, ENEMY_BAZOOKA, ENEMY_SHIELDED, ENEMY_GRENADE,
    // ENEMY_MARTIAN, ENEMY_PARATROOPER
    float x;
    float y;
    float landY;    // only used by ENEMY_PARATROOPER (landing Y); 0.f otherwise
};

// ─────────────────────────────────────────────────────────────────────────────
// Per-platform spawn entry — a horizontal row of blocks
// ─────────────────────────────────────────────────────────────────────────────
struct PlatformSpawnEntry {
    float startX;
    float startY;
    int   count;
};

// ─────────────────────────────────────────────────────────────────────────────
// LevelConfig — all data that changes between levels.
// PlayState reads from this struct instead of hardcoding values.
// When the player reaches the end of a level, PlayState increments
// currentLevelIndex and calls loadLevel() with the next config.
// ─────────────────────────────────────────────────────────────────────────────
struct LevelConfig {
    char bgPath[128];              // background image file path

    // ── Terrain ──
    bool  hasMountain;             // false = flat ground only (no mountain terrain)
    bool  visibleGround;           // false = invisible collision-only ground (no dirt sprites)
    bool  enableVerticalScroll;    // false = camera never scrolls vertically (flat level)
    bool  tileBg;                  // true = tile BG horizontally to cover full level width
    float levelWidth;              // 0.f = use grid width; > 0 = override play area width

    // ── Boss ──
    bool  isBossLevel;             // true = this level only spawns a boss, no regular enemies
    int   bossType;                // ENEMY_BOSS_IRONOKAVA, etc. (0 = no boss)

    // ── Water pool ──
    bool  hasWater;
    float waterX1, waterY1;        // top-left corner
    float waterX2, waterY2;        // bottom-right corner

    // ── Submarine ──
    bool  hasSubmarine;
    float submarineSpawnX;
    float submarineSpawnY;
    int   submarineDir;
    float submarineTriggerX;       // player X that triggers the spawn

    // ── FlyingTara ──
    int   flyingTaraPasses;        // 0 = none, 1 = one pass, 2 = two passes
    float flyingTaraTimes[4];      // seconds when each pass spawns (max 4)

    // ── Enemies ──
    int enemyCount;
    EnemySpawnEntry enemies[40];   // max 40 enemies per level

    // ── Platforms ──
    int platformCount;
    PlatformSpawnEntry platforms[10]; // max 10 platform groups per level
};

// ─────────────────────────────────────────────────────────────────────────────
// Three level configurations — one per stage
// ─────────────────────────────────────────────────────────────────────────────

// Level 1: Grassland — current level, normal difficulty (UNCHANGED)
static const LevelConfig LEVEL_1 = {
    "resources/Sprites/background.png",

    // Terrain
    true,           // hasMountain
    true,           // visibleGround
    true,           // enableVerticalScroll
    false,          // tileBg
    0.f,            // levelWidth (0 = use grid width)

    // Boss
    false,          // isBossLevel
    0,              // bossType

    // Water pool
    true,
    10242.f, 574.f,     // top-left
    115919.f, 1776.f,   // bottom-right

    // Submarine
    true,
    11000.f, 900.f,     // spawn position
    DIR_LEFT,           // initial direction
    9000.f,             // trigger: player X >= this

    // FlyingTara
    2,
    { 10.f, 16.f, 0.f, 0.f },

    // Enemies (13)
    13,
    {
        { ENEMY_MARTIAN,    15.f * 48.f, 0.f, 0.f },    // y filled at runtime
        { ENEMY_REBEL,      35.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      10.f * 48.f, 0.f, 0.f },    // platform enemy
        { ENEMY_REBEL,      24.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      34.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      48.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      0.f, 0.f, 0.f },           // mountain enemy (x filled at runtime)
        { ENEMY_REBEL,      0.f, 0.f, 0.f },           // mountain enemy
        { ENEMY_BAZOOKA,    25.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    50.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   20.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   40.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   0.f, 0.f, 0.f },           // mountain shielded
    },

    // Platforms (5)
    5,
    {
        {  8.f * 48.f, 32.f * 48.f, 8 },
        { 22.f * 48.f, 30.f * 48.f, 7 },
        { 32.f * 48.f, 30.f * 48.f, 4 },
        { 42.f * 48.f, 30.f * 48.f, 10 },
        { 0.f, 0.f, 0 },  // unused
    }
};

// Level 2: Desert — fewer enemies, earlier tara attacks
static const LevelConfig LEVEL_2 = {
    "resources/Sprites/background2.png",

    // Terrain
    true,           // hasMountain
    true,           // visibleGround
    true,           // enableVerticalScroll
    false,          // tileBg
    0.f,            // levelWidth (0 = use grid width)

    // Boss
    false,          // isBossLevel
    0,              // bossType

    // Water pool — same as level 1
    true,
    10242.f, 574.f,
    115919.f, 1776.f,

    // Submarine
    true,
    11000.f, 900.f,
    DIR_RIGHT,
    8000.f,             // spawns earlier

    // FlyingTara — 2 passes
    2,
    { 8.f, 16.f, 0.f, 0.f },

    // Enemies (8 — reduced)
    8,
    {
        { ENEMY_REBEL,      15.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      35.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    25.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    50.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   20.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   0.f, 0.f, 0.f },       // mountain enemy
        { ENEMY_MARTIAN,    40.f * 48.f, 0.f, 0.f },
        { ENEMY_GRENADE,    30.f * 48.f, 0.f, 0.f },
    },

    // Platforms (4)
    4,
    {
        {  8.f * 48.f, 32.f * 48.f, 8 },
        { 22.f * 48.f, 30.f * 48.f, 7 },
        { 42.f * 48.f, 30.f * 48.f, 10 },
        { 55.f * 48.f, 30.f * 48.f, 6 },
    }
};

// Level 3: Plains — 11000px wide, flat, no water, invisible ground, tiled BG
static const LevelConfig LEVEL_3 = {
    "resources/Sprites/background3.png",

    // Terrain — flat plains, no mountain, invisible ground, tiled BG, no vertical zoom
    false,          // hasMountain
    false,          // visibleGround  — invisible collision-only ground
    false,          // enableVerticalScroll
    true,           // tileBg — tile BG to cover full 11000px width
    11000.f,        // levelWidth — 11000px play area

    // Boss
    false,          // isBossLevel
    0,              // bossType

    // Water pool — none
    false,
    0.f, 0.f,
    0.f, 0.f,

    // Submarine — none
    false,
    0.f, 0.f,
    DIR_LEFT,
    0.f,

    // FlyingTara — 4 passes, spread across the level
    4,
    { 5.f, 12.f, 22.f, 35.f },

    // Enemies (18 — evenly distributed across 11000px, more variety)
    18,
    {
        // ── Early section (0-3000px) ──
        { ENEMY_REBEL,       600.f, 0.f, 0.f },
        { ENEMY_REBEL,      1200.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    1800.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   2400.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,2800.f, 0.f, 0.f },

        // ── Mid section (3000-6000px) ──
        { ENEMY_REBEL,      3500.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    4200.f, 0.f, 0.f },
        { ENEMY_MARTIAN,    4800.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   5200.f, 0.f, 0.f },
        { ENEMY_GRENADE,    5600.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,5900.f, 0.f, 0.f },

        // ── Late section (6000-9000px) ──
        { ENEMY_REBEL,      6500.f, 0.f, 0.f },
        { ENEMY_MARTIAN,    7100.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    7600.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   8200.f, 0.f, 0.f },
        { ENEMY_GRENADE,    8700.f, 0.f, 0.f },

        // ── Final stretch (9000-11000px) ──
        { ENEMY_MARTIAN,    9400.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,   10000.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,10500.f, 0.f, 0.f },
    },

    // Platforms (5 — spread across the wider level)
    5,
    {
        { 1500.f, 30.f * 48.f, 6 },
        { 4000.f, 30.f * 48.f, 5 },
        { 6500.f, 30.f * 48.f, 6 },
        { 8500.f, 30.f * 48.f, 5 },
        {10000.f, 30.f * 48.f, 4 },
    }
};

// Level 4: Ironokava Boss Arena — same BG/width as level 3, no enemies, only the boss
static const LevelConfig LEVEL_4 = {
    "resources/Sprites/background3.png",

    // Terrain — same as level 3: flat, no mountain, invisible ground, tiled BG
    false,          // hasMountain
    false,          // visibleGround
    false,          // enableVerticalScroll
    true,           // tileBg
    11000.f,        // levelWidth — same width as level 3

    // Boss
    true,           // isBossLevel
    ENEMY_BOSS_IRONOKAVA,  // bossType

    // Water pool — none
    false,
    0.f, 0.f,
    0.f, 0.f,

    // Submarine — none
    false,
    0.f, 0.f,
    DIR_LEFT,
    0.f,

    // FlyingTara — none (boss level)
    0,
    { 0.f, 0.f, 0.f, 0.f },

    // Enemies — none (boss level, boss spawned separately)
    0,
    { },

    // Platforms — none (open boss arena)
    0,
    { }
};

// Array of all level configs — indexed by level number (0-based)
static const LevelConfig* ALL_LEVELS[4] = {
    &LEVEL_1,
    &LEVEL_2,
    &LEVEL_3,
    &LEVEL_4
};

static const int TOTAL_LEVELS = 4;

// ─────────────────────────────────────────────────────────────────────────────
// Game state ID constants
// ─────────────────────────────────────────────────────────────────────────────
static const int GSTATE_LEVEL_SELECT = 20;
