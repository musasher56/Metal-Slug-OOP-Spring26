#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace sf;




const int STATE_HEALTHY = 0;
const int STATE_INJURED = 1;
const int STATE_CRITICAL = 2;
const int STATE_DEAD = 3;
const int STATE_UNDEAD = 4;
const int STATE_MUMMY = 5;


const int DIR_LEFT = 0;
const int DIR_RIGHT = 1;


const int BIOME_AERIAL = 0;
const int BIOME_PLAINS = 1;
const int BIOME_AQUATIC = 2;

// Biome height thresholds — used by Level::generateColumn() and getBiomeAt()
// Fractal noise outputs [0.0, 1.0]:
//   > BIOME_AERIAL_THRESHOLD  = aerial (tall peaks, rocky grey terrain)
//   < BIOME_AQUATIC_THRESHOLD = aquatic (deep valleys, water fills above surface)
//   everything in between     = plains (standard brown earth with grass)
const float BIOME_AERIAL_THRESHOLD = 0.70f;
const float BIOME_AQUATIC_THRESHOLD = 0.30f;


const int VEHICLE_GROUND = 0;
const int VEHICLE_AERIAL = 1;
const int VEHICLE_AQUATIC = 2;


const int BEHAVIOR_PRESSING = 0;
const int BEHAVIOR_CONSOLIDATING = 1;
const int BEHAVIOR_RETREATING = 2;
const int BEHAVIOR_BURNING_RESOURCES = 3;
const int BEHAVIOR_STALLING = 4;


const int ENEMY_REBEL = 0;
const int ENEMY_SHIELDED = 1;
const int ENEMY_BAZOOKA = 2;
const int ENEMY_GRENADE = 3;
const int ENEMY_ZOMBIE = 4;
const int ENEMY_MUMMY_WARRIOR = 5;
const int ENEMY_PARATROOPER = 6;
const int ENEMY_MARTIAN = 7;
const int ENEMY_FLYING_TARA = 8;
const int ENEMY_BRADLEY = 9;
const int ENEMY_SUB = 10;
const int ENEMY_BOSS_IRONOKAVA = 11;
const int ENEMY_BOSS_HAIRBUSTER = 12;


const int NOISE_AMPLIFIED = 0;
const int NOISE_FLAT = 1;
const int NOISE_NORMAL = 2;


const int MODE_SURVIVAL = 0;
const int MODE_CAMPAIGN = 1;
const int MODE_SELF_PLAY = 2;


const int WEAPON_PISTOL = 0;
const int WEAPON_HMG = 1;
const int WEAPON_ROCKET_LAUNCHER = 2;
const int WEAPON_FLAME_SHOT = 3;
const int WEAPON_LASER_GUN = 4;


const int AI_IDLE = 0;
const int AI_PATROL = 1;
const int AI_CHASE = 2;
const int AI_ATTACK = 3;
const int AI_RETREAT = 4;
const int AI_SPECIAL = 5;

// Boss AI states
const int AI_BOSS_IDLE = 10;
const int AI_BOSS_WALK = 11;
const int AI_BOSS_ATTACK = 12;
const int AI_BOSS_CHARGE = 13;
const int AI_BOSS_SPECIAL = 14;


const int TRANSFORM_NONE = 0;
const int TRANSFORM_UNDEAD = 1;
const int TRANSFORM_MUMMY = 2;


const int PROJ_STRAIGHT = 0;
const int PROJ_BALLISTIC = 1;
const int PROJ_EXPLOSIVE = 2;
const int PROJ_BEAM = 3;
const int PROJ_FLAME = 5;   // FlameShot stream particle — short lifetime


const int GSTATE_MENU = 0;
const int GSTATE_PLAY = 1;
const int GSTATE_PAUSED = 2;
const int GSTATE_GAME_OVER = 3;
const int GSTATE_LEADERBOARD = 4;
const int GSTATE_CHAR_SELECT = 5;   // Character select screen (between menu and PlayState)


const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int FRAMERATE_LIMIT = 60;

const int MAX_SOUNDS = 50;
const int MAX_TEXTURES = 100;
const int MAX_NAME_LEN = 64;
const int MAX_ENTITIES = 200;
const int ENTITY_POOL_GROWTH = 100;


const int HITS_PER_HEART = 3;
const int PROJ_BOMB = 4;

// ─────────────────────────────────────────────────────────────────────────────
// Level grid dimensions
// ─────────────────────────────────────────────────────────────────────────────

// Cell size in pixels — every grid cell is 48×48 px
const int CELL_SIZE = 48;

// Survival mode grid: 40 rows × 240 cols
// 40 rows * 48px = 1920px total height (with vertical scroll)
// 240 cols * 48px = 11520px total width
const int SURVIVAL_HEIGHT = 40;
const int SURVIVAL_WIDTH = 240;

// Campaign mode grid: 50 rows × 420 cols
// 50 rows * 48px = 2400px total height (room for amplified peaks up to ~25 blocks)
// 420 cols * 48px = 20160px visible window, infinite via advanceWorld/retreatWorld
const int CAMPAIGN_HEIGHT = 50;
const int CAMPAIGN_WIDTH = 420;
