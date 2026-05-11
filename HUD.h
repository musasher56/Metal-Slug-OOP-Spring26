#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class CharacterManager;
class ScoreManager;

class HUD {
private:
    int          hp;
    int          maxHp;
    Font         font;
    float        redHueAlpha;

    Texture      heartTex[4];
    bool         heartsLoaded;

    int          currentScore;
    int          highScore;

    int          ammo;
    bool         infiniteAmmo;
    const char*  weaponName;
    int          grenadeCount;

    float        bossHealthFraction;      
    float        bossHealthDisplayed;     
    const char* bossName;               
    bool         bossBarVisible;          
    float        bossBarAppearTimer;      
    float        bossBarAlpha;            

    bool         felledVisible;           
    int          felledPhase;             
    float        felledTimer;             
    float        felledAlpha;             
    const char* felledBossName;          

public:
    HUD();
    virtual ~HUD();

    void update(CharacterManager* cm, int levelNum);
    void draw(RenderWindow& window);
    void showDamageHue(float intensity);

    void setScore(int score);
    void setHighScore(int hs);

    void setBossInfo(const char* name, float healthFrac);
    void clearBossInfo();

    void showBossFelled(const char* bossName);
    bool isFelledShowing() const;

private:
    void drawBossFelled(RenderWindow& window);
};
