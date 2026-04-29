#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class PlayerSoldier;

// WHY: ScoreManager tracks score, combo multipliers, and kill bonuses
class ScoreManager {
private:
    int score;
    float comboMultiplier;
    Clock lastKillTime;

public:
    ScoreManager();
    virtual ~ScoreManager();
    
    void addKill(int enemyType, bool isAerial, bool isMelee);
    void addMultiKill(int count, bool isGrenade);
    void addBossKill(bool isUltimate);
    void addLevelClearBonus(int mode, bool flawless);
    int getScore() const;
    void reset();
};
