#include "ScoreManager.h"

ScoreManager::ScoreManager() : score(0), highScore(0), comboMultiplier(1.0f) {
}

ScoreManager::~ScoreManager() {
}

void ScoreManager::addKill(int enemyType, bool isAerial, bool isMelee) {
    
    int baseScore = 50;
    if (enemyType == ENEMY_REBEL) {
        baseScore = 50;
    } else if (enemyType == ENEMY_SHIELDED) {
        baseScore = 75;
    } else if (enemyType == ENEMY_BAZOOKA) {
        baseScore = 100;
    } else if (enemyType == ENEMY_GRENADE) {
        baseScore = 100;
    } else if (enemyType == ENEMY_ZOMBIE) {
        baseScore = 100;
    } else if (enemyType == ENEMY_MUMMY_WARRIOR) {
        baseScore = 150;
    } else if (enemyType == ENEMY_PARATROOPER) {
        baseScore = 75;
    } else if (enemyType == ENEMY_MARTIAN) {
        baseScore = 200;
    }
    
    if (isAerial) baseScore += 100;
    if (isMelee) baseScore += 50;
    
    this->score += (int)(baseScore * this->comboMultiplier);
}

void ScoreManager::addMultiKill(int count, bool isGrenade) {
    int bonus = 300 + (count * 50);
    if (isGrenade) bonus += 200;
    this->score += bonus;
}

void ScoreManager::addBossKill(bool isUltimate) {
    if (isUltimate) {
        this->score += 1500;
    } else {
        this->score += 500;
    }
}

void ScoreManager::addLevelClearBonus(int mode, bool flawless) {
    if (mode == MODE_SURVIVAL) {
        this->score += 1000;
    } else if (mode == MODE_CAMPAIGN) {
        this->score += 3000;
    }
    if (flawless) {
        this->score += 5000;
    }
}

void ScoreManager::updateHighScore() {
    if (this->score > this->highScore) {
        this->highScore = this->score;
    }
}

int ScoreManager::getScore() const {
    return this->score;
}

int ScoreManager::getHighScore() const {
    return this->highScore;
}

void ScoreManager::reset() {
    this->updateHighScore();
    this->score = 0;
    this->comboMultiplier = 1.0f;
}
