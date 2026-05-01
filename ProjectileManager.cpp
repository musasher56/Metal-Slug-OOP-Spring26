#include "ProjectileManager.h"
#include "Level.h"
#include <cmath>

ProjectileManager::ProjectileManager(TextureManager* t, AudioManager* a)
    : activeCount(0), texMgr(t), audMgr(a)
{
    for (int i = 0; i < MAX_PROJ; i++) this->slots[i] = nullptr;
}

ProjectileManager::~ProjectileManager() { this->clearAll(); }

void ProjectileManager::clearAll() {
    for (int i = 0; i < this->activeCount; i++) {
        delete this->slots[i];
        this->slots[i] = nullptr;
    }
    this->activeCount = 0;
}

// ============================================================
// calcBarrelTip  (static)
// ============================================================
// WHERE DO PROJECTILES SPAWN FROM?
//
// Metal Slug bullets come from the barrel tip of the weapon,
// NOT from the player's origin (top-left of sprite).
//
// Visual layout for a 32×64px player sprite facing right:
//
//   pos.x  pos.x+32
//     │        │
//     ┌────────┐  ← pos.y
//     │  head  │
//     │  ●─────┼──►  ← pos.y + barrelOffsetY  (e.g. 20px from top)
//     │  body  │
//     └────────┘
//                ↑
//           spawn here (pos.x + spriteWidth + GAP)
//
// Facing left: mirror — spawn at pos.x - GAP
//
// barrelOffsetY: vertical distance from pos.y to where the gun sits.
// For a 64px tall player, ~20px puts the barrel at upper-body height.
// Fine-tune this once the character sprite is visible.
// ============================================================
sf::Vector2f ProjectileManager::calcBarrelTip(sf::Vector2f entityPos,
                                               int dir,
                                               float spriteWidth,
                                               float barrelOffsetY)
{
    const float GAP = 4.f;  // clearance so bullet starts outside hitbox

    float x = (dir == DIR_RIGHT)
               ? entityPos.x + spriteWidth + GAP
               : entityPos.x - GAP;

    float y = entityPos.y + barrelOffsetY;
    return sf::Vector2f(x, y);
}

// ============================================================
// angleToVelocity  (static)
// ============================================================
// angle=0  → horizontal  (velX = ±speed, velY = 0)
// angle=90 → straight up (velX = 0,      velY = -speed)
// Screen Y-axis is inverted, so "up" = negative velY.
// ============================================================
void ProjectileManager::angleToVelocity(float angle, int dir, float speed,
                                         float& outVX, float& outVY)
{
    float rad  = angle * 3.14159f / 180.f;
    outVX = (dir == DIR_RIGHT ? 1.f : -1.f) * cosf(rad) * speed;
    outVY = -sinf(rad) * speed;
}

// ============================================================
// spawnStraight — pistol / HMG bullets
// ============================================================
void ProjectileManager::spawnStraight(sf::Vector2f origin, int dir,
                                       float angle, int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    StraightProjectile* p = new StraightProjectile(this->texMgr, this->audMgr, angle);
    p->position  = origin;
    p->fromEnemy = fromEnemy;
    p->damage    = dmg;

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 15.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

// ============================================================
// spawnExplosive — rocket launcher
// ============================================================
void ProjectileManager::spawnExplosive(sf::Vector2f origin, int dir,
                                        float angle, int dmg,
                                        int blastRadius, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position    = origin;
    p->fromEnemy   = fromEnemy;
    p->damage      = dmg;
    p->blastRadius = blastRadius;

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 8.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

// ============================================================
// update
// ============================================================
void ProjectileManager::update(float scroll, Level* lvl) {
    int i = 0;
    while (i < this->activeCount) {
        Projectile* p = this->slots[i];
        if (p == nullptr) { this->removeAt(i); continue; }

        p->update(scroll, lvl);

        if (!p->getStatus()) this->removeAt(i);
        else                 i++;
    }
}

// ============================================================
// draw — PLACEHOLDER coloured rectangles
// ============================================================
// Renders coloured 8×8 squares so you can SEE bullets moving and
// debug spawn origin / angle / speed before the sprite sheet is ready.
//
//   Yellow  = straight projectile (pistol / HMG bullet)
//   Orange  = explosive projectile (rocket)
//
// TO SWAP IN REAL SPRITES later:
//   1. In spawnStraight(), after new StraightProjectile:
//        p->animation.setTexture(&texMgr->getTexture("bullets.png"));
//        p->animation.setFrameCount(1);
//        p->sprite.setTextureRect(IntRect(X, Y, W, H)); // your sheet coords
//   2. Replace the RectangleShape block below with:
//        p->draw(window, scroll);
// ============================================================
void ProjectileManager::draw(RenderWindow& window, float scroll) {
    RectangleShape rect(sf::Vector2f(8.f, 8.f));

    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->getStatus()) continue;

        rect.setPosition(p->position.x - scroll, p->position.y);

        // WHY reuse one RectangleShape object instead of creating per bullet?
        // RectangleShape allocation inside a loop creates/destroys a heap
        // object every frame per bullet.  One shape, reset each iteration.
        if (p->isExplosive) {
            rect.setFillColor(Color(255, 140, 0));   // orange — rocket
        } else {
            rect.setFillColor(Color(255, 255, 0));   // yellow — bullet
        }

        window.draw(rect);
    }
}

// ============================================================
// checkEntityCollisions
// ============================================================
int ProjectileManager::checkEntityCollisions(DamagableEntity** targets,
                                              int targetCount)
{
    int totalDamage = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();
        bool    hit     = false;

        for (int e = 0; e < targetCount && !hit; e++) {
            if (targets[e] == nullptr || !targets[e]->isAlive()) continue;

            IntRect entBox = targets[e]->getBoundingBox();

            bool overlapX = (projBox.left < entBox.left + entBox.width)  &&
                            (projBox.left + projBox.width  > entBox.left);
            bool overlapY = (projBox.top  < entBox.top  + entBox.height) &&
                            (projBox.top  + projBox.height > entBox.top);

            if (overlapX && overlapY) {
                targets[e]->takeDamage(proj->getDamage());
                totalDamage += proj->getDamage();
                proj->onImpact(nullptr, nullptr);
                proj->deactivate();
                hit = true;
            }
        }

        if (!proj->getStatus()) this->removeAt(p);
        else                    p++;
    }

    return totalDamage;
}

// ============================================================
// removeAt — O(1) swap-remove
// ============================================================
void ProjectileManager::removeAt(int i) {
    delete this->slots[i];
    this->slots[i] = nullptr;
    this->activeCount--;

    if (i < this->activeCount) {
        this->slots[i] = this->slots[this->activeCount];
        this->slots[this->activeCount] = nullptr;
    }
}