#pragma once
#include "Projectile.h"
#include "DamagableEntity.h"

class Level;

class ProjectileManager {
public:
    static const int MAX_PROJ = 64;
    static const int MAX_BLASTS = 16;

private:
    Projectile* slots[MAX_PROJ];
    int             activeCount;
    TextureManager* texMgr;
    AudioManager* audMgr;

    // Blast effect pool
    struct BlastEffect {
        Animation anim;
        float x;
        float y;
        bool active;
    };
    BlastEffect blasts[MAX_BLASTS];
    int blastCount;

public:
    ProjectileManager(TextureManager* t, AudioManager* a);
    ~ProjectileManager();
    void spawnBomb(sf::Vector2f origin, int dir, float angle,
        int dmg, int blastRadius, bool fromEnemy = false);
    static sf::Vector2f calcBarrelTip(sf::Vector2f entityPos,
        int dir,
        float spriteWidth,
        float barrelOffsetY);

    void spawnStraight(sf::Vector2f origin, int dir, float angle,
        int dmg, bool fromEnemy = false);

    void spawnExplosive(sf::Vector2f origin, int dir, float angle,
        int dmg, int blastRadius, bool fromEnemy = false);

    void spawnBlast(float x, float y);

    void update(float scroll, Level* lvl);

    void postEntityUpdate(float scrollX, float scrollY, Level* lvl);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    int  checkEntityCollisions(DamagableEntity** targets, int targetCount);
    int  checkPlayerBulletHits(DamagableEntity** targets, int targetCount);
    bool checkEnemyBulletHitPlayer(DamagableEntity* player);

    int  getActiveCount() const { return this->activeCount; }
    Projectile** getSlots() { return this->slots; }
    void clearAll();

private:
    void removeAt(int i);

    static void angleToVelocity(float angle, int dir, float speed,
        float& outVX, float& outVY);
};