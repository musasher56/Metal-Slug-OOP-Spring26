#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class PlayerSoldier;


class ScoreManager {
private:
    int score;
    int highScore;
    float comboMultiplier;
    Clock lastKillTime;

    void updateHighScore();

public:
    ScoreManager();
    virtual ~ScoreManager();
    
    void addKill(int enemyType, bool isAerial, bool isMelee);
    void addMultiKill(int count, bool isGrenade);
    void addBossKill(bool isUltimate);
    void addLevelClearBonus(int mode, bool flawless);
    int getScore() const;
    int getHighScore() const;
    void reset();
};
