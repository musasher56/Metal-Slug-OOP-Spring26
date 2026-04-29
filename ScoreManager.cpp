#include "ScoreManager.h"

ScoreManager::ScoreManager() : score(0), comboMultiplier(1.0f) {
}

ScoreManager::~ScoreManager() {
}

void ScoreManager::addKill(int enemyType, bool isAerial, bool isMelee) {
    // WHY: Add base score for enemy type
    int baseScore = 50;  // Default Rebel score
    switch (enemyType) {
        case ENEMY_REBEL: baseScore = 50; break;
        case ENEMY_SHIELDED: baseScore = 75; break;
        case ENEMY_BAZOOKA: baseScore = 100; break;
        case ENEMY_GRENADE: baseScore = 100; break;
        case ENEMY_ZOMBIE: baseScore = 100; break;
        case ENEMY_MUMMY_WARRIOR: baseScore = 150; break;
        case ENEMY_PARATROOPER: baseScore = 75; break;  // base + 25
        case ENEMY_MARTIAN: baseScore = 200; break;
        default: break;
    }
    
    if (isAerial) baseScore += 100;
    if (isMelee) baseScore += 50;
    
    this->score += (int)(baseScore * this->comboMultiplier);
}

void ScoreManager::addMultiKill(int count, bool isGrenade) {
    // WHY: Multi-kill bonus
    int bonus = 300 + (count * 50);
    if (isGrenade) bonus += 200;
    this->score += bonus;
}

void ScoreManager::addBossKill(bool isUltimate) {
    // WHY: Boss kill bonus
    if (isUltimate) {
        this->score += 1500;
    } else {
        this->score += 500;  // Per phase
    }
}

void ScoreManager::addLevelClearBonus(int mode, bool flawless) {
    // WHY: Level clear bonus
    if (mode == MODE_SURVIVAL) {
        this->score += 1000;
    } else if (mode == MODE_CAMPAIGN) {
        this->score += 3000;
    }
    if (flawless) {
        this->score += 5000;
    }
}

int ScoreManager::getScore() const {
    return this->score;
}

void ScoreManager::reset() {
    this->score = 0;
    this->comboMultiplier = 1.0f;
}
