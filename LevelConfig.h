#pragma once
#include "Constants.h"

struct EnemySpawnEntry {
    int   type;     
    float x;
    float y;
    float landY;
};
struct PlatformSpawnEntry {
    float startX;
    float startY;
    int   count;
};

struct LevelConfig {
    char bgPath[128];          

    
    bool  hasMountain;             
    bool  visibleGround;           
    bool  enableVerticalScroll;    
    bool  tileBg;                  
    float levelWidth;              

    bool  isBossLevel;            
    int   bossType;              

    bool  hasWater;
    float waterX1, waterY1;      
    float waterX2, waterY2;       

    bool  hasSubmarine;
    float submarineSpawnX;
    float submarineSpawnY;
    int   submarineDir;
    float submarineTriggerX;    
   
    int   flyingTaraPasses;        
    float flyingTaraTimes[4];    

    int enemyCount;
    EnemySpawnEntry enemies[40];   

    int platformCount;
    PlatformSpawnEntry platforms[10];
};

static const LevelConfig CAMPAIGN_LEVEL = {
    "resources/Sprites/infinite.png",

    
    false,         
    false,        
    true,          
    true,           
    0.f,     

    false,        
    0,       

    false,
    0.f, 0.f,
    0.f, 0.f,

    false,
    0.f, 0.f,
    DIR_LEFT,
    0.f,

    2,
    { 10.f, 25.f, 0.f, 0.f },

    0,
    { },

    0,
    { }
};

static const LevelConfig LEVEL_1 = {
    "resources/Sprites/background.png",

    true,           
    true,          
    true,           
    false,         
    0.f,           

   
    false,       
    0,           

    true,
    10242.f, 574.f,     
    115919.f, 1776.f,  

    true,
    11000.f, 900.f,     // spawn position
    DIR_LEFT,          
    9000.f,           

    // FlyingTara
    2,
    { 10.f, 16.f, 0.f, 0.f },

    // Enemies (13)
    13,
    {
        { ENEMY_MARTIAN,    15.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      35.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      10.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      24.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      34.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      48.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      0.f, 0.f, 0.f },
        { ENEMY_REBEL,      0.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    25.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    50.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   20.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   40.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   0.f, 0.f, 0.f },
    },

    // Platforms (5)
    5,
    {
        {  8.f * 48.f, 32.f * 48.f, 8 },
        { 22.f * 48.f, 30.f * 48.f, 7 },
        { 32.f * 48.f, 30.f * 48.f, 4 },
        { 42.f * 48.f, 30.f * 48.f, 10 },
        { 0.f, 0.f, 0 },
    }
};


static const LevelConfig LEVEL_2 = {
    "resources/Sprites/background2.png",

    true,
    true,      
    true,         
    false,         
    0.f,            

    false,         
    0,          

    true,
    10242.f, 574.f,
    115919.f, 1776.f,

    true,
    11000.f, 900.f,
    DIR_RIGHT,
    8000.f,

    2,
    { 8.f, 16.f, 0.f, 0.f },

    8,
    {
        { ENEMY_REBEL,      15.f * 48.f, 0.f, 0.f },
        { ENEMY_REBEL,      35.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    25.f * 48.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    50.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   20.f * 48.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   0.f, 0.f, 0.f },
        { ENEMY_MARTIAN,    40.f * 48.f, 0.f, 0.f },
        { ENEMY_GRENADE,    30.f * 48.f, 0.f, 0.f },
    },

    4,
    {
        {  8.f * 48.f, 32.f * 48.f, 8 },
        { 22.f * 48.f, 30.f * 48.f, 7 },
        { 42.f * 48.f, 30.f * 48.f, 10 },
        { 55.f * 48.f, 30.f * 48.f, 6 },
    }
};

static const LevelConfig LEVEL_3 = {
    "resources/Sprites/background3.png",

    false,         
    false,        
    false,         
    true,           
    11000.f,    

    false,         
    0,            
    false,
    0.f, 0.f,
    0.f, 0.f,
    false,
    0.f, 0.f,
    DIR_LEFT,
    0.f,

    4,
    { 5.f, 12.f, 22.f, 35.f },
    18,
    {
        { ENEMY_REBEL,       600.f, 0.f, 0.f },
        { ENEMY_REBEL,      1200.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    1800.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   2400.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,2800.f, 0.f, 0.f },

        { ENEMY_REBEL,      3500.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    4200.f, 0.f, 0.f },
        { ENEMY_MARTIAN,    4800.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   5200.f, 0.f, 0.f },
        { ENEMY_GRENADE,    5600.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,5900.f, 0.f, 0.f },

        { ENEMY_REBEL,      6500.f, 0.f, 0.f },
        { ENEMY_MARTIAN,    7100.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,    7600.f, 0.f, 0.f },
        { ENEMY_SHIELDED,   8200.f, 0.f, 0.f },
        { ENEMY_GRENADE,    8700.f, 0.f, 0.f },

        { ENEMY_MARTIAN,    9400.f, 0.f, 0.f },
        { ENEMY_BAZOOKA,   10000.f, 0.f, 0.f },
        { ENEMY_PARATROOPER,10500.f, 0.f, 0.f },
    },

    // Platforms (5)
    5,
    {
        { 1500.f, 30.f * 48.f, 6 },
        { 4000.f, 30.f * 48.f, 5 },
        { 6500.f, 30.f * 48.f, 6 },
        { 8500.f, 30.f * 48.f, 5 },
        {10000.f, 30.f * 48.f, 4 },
    }
};

static const LevelConfig LEVEL_4 = {
    "resources/Sprites/background3.png",

    false,          
    false,        
    false,          
    true,           
    11000.f,     

    true,          
    ENEMY_BOSS_IRONOKAVA,

    true,
    8350.f, 1450.f,
    10125.f, 2256.f,

    false,
    0.f, 0.f,
    DIR_LEFT,
    0.f,

    0,
    { 0.f, 0.f, 0.f, 0.f },

    0,
    { },

    0,
    { }
};

static const LevelConfig* ALL_LEVELS[4] = {
    &LEVEL_1,
    &LEVEL_2,
    &LEVEL_3,
    &LEVEL_4
};

static const int TOTAL_LEVELS = 4;

static const int GSTATE_LEVEL_SELECT = 20;
