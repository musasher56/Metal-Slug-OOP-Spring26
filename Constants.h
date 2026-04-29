#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace sf;

// ========== INTEGER STATE CONSTANTS ==========

// CharacterState / TransformationState
const int STATE_HEALTHY   = 0;
const int STATE_INJURED   = 1;
const int STATE_CRITICAL  = 2;
const int STATE_DEAD      = 3;
const int STATE_UNDEAD    = 4;
const int STATE_MUMMY     = 5;

// Direction
const int DIR_LEFT  = 0;
const int DIR_RIGHT = 1;

// BiomeType
const int BIOME_AERIAL   = 0;
const int BIOME_PLAINS   = 1;
const int BIOME_AQUATIC  = 2;

// VehicleMode
const int VEHICLE_GROUND   = 0;
const int VEHICLE_AERIAL   = 1;
const int VEHICLE_AQUATIC  = 2;

// BehaviorState
const int BEHAVIOR_PRESSING         = 0;
const int BEHAVIOR_CONSOLIDATING    = 1;
const int BEHAVIOR_RETREATING       = 2;
const int BEHAVIOR_BURNING_RESOURCES = 3;
const int BEHAVIOR_STALLING         = 4;

// EnemyType
const int ENEMY_REBEL          = 0;
const int ENEMY_SHIELDED       = 1;
const int ENEMY_BAZOOKA        = 2;
const int ENEMY_GRENADE        = 3;
const int ENEMY_ZOMBIE         = 4;
const int ENEMY_MUMMY_WARRIOR  = 5;
const int ENEMY_PARATROOPER    = 6;
const int ENEMY_MARTIAN        = 7;
const int ENEMY_FLYING_TARA    = 8;
const int ENEMY_BRADLEY        = 9;
const int ENEMY_SUB            = 10;

// NoiseProfileType
const int NOISE_AMPLIFIED  = 0;
const int NOISE_FLAT       = 1;
const int NOISE_NORMAL     = 2;

// GameMode
const int MODE_SURVIVAL   = 0;
const int MODE_CAMPAIGN   = 1;
const int MODE_SELF_PLAY  = 2;

// WeaponType
const int WEAPON_PISTOL           = 0;
const int WEAPON_HMG              = 1;
const int WEAPON_ROCKET_LAUNCHER  = 2;
const int WEAPON_FLAME_SHOT       = 3;
const int WEAPON_LASER_GUN        = 4;

// AIStateType
const int AI_IDLE     = 0;
const int AI_PATROL   = 1;
const int AI_CHASE    = 2;
const int AI_ATTACK   = 3;
const int AI_RETREAT  = 4;
const int AI_SPECIAL  = 5;

// TransformationType
const int TRANSFORM_NONE    = 0;
const int TRANSFORM_UNDEAD  = 1;
const int TRANSFORM_MUMMY   = 2;

// ProjectileClass
const int PROJ_STRAIGHT   = 0;
const int PROJ_BALLISTIC  = 1;
const int PROJ_EXPLOSIVE  = 2;
const int PROJ_BEAM       = 3;

// GameStateID
const int STATE_MENU        = 0;
const int STATE_PLAY        = 1;
const int STATE_PAUSED      = 2;
const int STATE_GAME_OVER   = 3;
const int STATE_LEADERBOARD = 4;

// ========== SCREEN & LIMITS ==========
const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const int FRAMERATE_LIMIT = 60;

const int MAX_SOUNDS = 50;
const int MAX_TEXTURES = 100;
const int MAX_NAME_LEN = 64;
const int MAX_ENTITIES = 200;
const int ENTITY_POOL_GROWTH = 100;