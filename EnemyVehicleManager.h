#pragma once
#include "EnemyVehicle.h"

class PlayerSoldier;
class Level;

class EnemyVehicleManager {
public:
    static const int MAX_VEHICLES = 16;

private:
    EnemyVehicle* slots[MAX_VEHICLES];
    DamagableEntity* deSlots[MAX_VEHICLES];
    int activeCount;
    TextureManager* texMgr;
    AudioManager* audMgr;
    ProjectileManager* pm;

public:
    EnemyVehicleManager(TextureManager* t, AudioManager* a);
    ~EnemyVehicleManager();

    void setProjectileManager(ProjectileManager* p);

    int spawnFlyingTara(float x, float y, int dir);
    int spawnSubmarine(float x, float y, int dir);

    void update(float scroll, float scrollY, Level* lvl,
        PlayerSoldier* player, ProjectileManager* projMgr);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    DamagableEntity** getDamagableSlots();
    int getActiveCount() const;

    void cleanup();
    void clearAll();

private:
    int findFreeSlot();
    void removeAt(int i);
};