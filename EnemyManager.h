#pragma once
#include "Enemy.h"

class PlayerSoldier;
class ScoreManager;

class EnemyManager {
public:
    static const int MAX_ENEMIES = 64;

private:
    Enemy* slots[MAX_ENEMIES];
    DamagableEntity* deSlots[MAX_ENEMIES];
    int activeCount;
    TextureManager* texMgr;
    AudioManager* audMgr;
    ProjectileManager* pm;
    ScoreManager* scoreMgr;
    int totalKills;
    Boss* activeBoss;
    bool bossDied;
    const char* bossDiedName;

public:
    EnemyManager(TextureManager* t, AudioManager* a);
    ~EnemyManager();

    void setProjectileManager(ProjectileManager* p);
    void setScoreManager(ScoreManager* sm);

    int spawnRebel(float x, float y);
    int spawnBazooka(float x, float y);
    int spawnShielded(float x, float y);
    int spawnGrenade(float x, float y);
    int spawnMartian(float x, float y);
    int spawnParatrooper(float x, float y, float landY);
    int spawnIronokava(float x, float y);
    int spawnHairbuster(float x, float y, float cx, float cy);
    int spawnSeaSatan(float x, float y, float cx, float cy, float surfY);
    int spawnSherry(float x, float y);

    void update(float scrollX, float scrollY, Level* lvl, PlayerSoldier* player);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    DamagableEntity** getDamagableSlots();
    int getActiveCount() const;
    int getTotalKills() const;

    Boss* getActiveBoss() const;
    bool hasActiveBoss() const;
    bool isBossDead() const;
    bool wasBossKilled() const;
    const char* getBossDiedName() const;
    void resetBossDied();

    void cleanup();
    void clearAll();

private:
    int findFreeSlot();
    void removeAt(int i);
};
