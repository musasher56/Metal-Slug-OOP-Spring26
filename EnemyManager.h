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

    void update(float scrollX, float scrollY, Level* lvl, PlayerSoldier* player);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    DamagableEntity** getDamagableSlots();
    int getActiveCount() const;
    int getTotalKills() const;

    void cleanup();
    void clearAll();

private:
    int findFreeSlot();
    void removeAt(int i);
};