#pragma once
#include "Enemy.h"

class PlayerSoldier;

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
    Boss* activeBoss;          // pointer to the current boss (if any)
    bool bossDied;             // true once the boss has been fully removed
    const char* bossDiedName;  // saved boss name for HUD felled message

public:
    EnemyManager(TextureManager* t, AudioManager* a);
    ~EnemyManager();

    void setProjectileManager(ProjectileManager* p);

    int spawnRebel(float x, float y);
    int spawnBazooka(float x, float y);
    int spawnShielded(float x, float y);
    int spawnGrenade(float x, float y);
    int spawnMartian(float x, float y);
    int spawnParatrooper(float x, float y, float landY);
    int spawnIronokava(float x, float y);

    void update(float scrollX, float scrollY, Level* lvl, PlayerSoldier* player);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    DamagableEntity** getDamagableSlots();
    int getActiveCount() const;
    int getTotalKills() const;

    Boss* getActiveBoss() const;
    bool hasActiveBoss() const;
    bool isBossDead() const;
    bool wasBossKilled() const;          // true after boss is fully removed from slots
    const char* getBossDiedName() const; // name of the boss that died

    void cleanup();
    void clearAll();

private:
    int findFreeSlot();
    void removeAt(int i);
};