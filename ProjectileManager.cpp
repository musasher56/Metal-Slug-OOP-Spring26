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

sf::Vector2f ProjectileManager::calcBarrelTip(sf::Vector2f entityPos,
    int dir,
    float spriteWidth,
    float barrelOffsetY)
{
    const float GAP = 4.f;

    float x = (dir == DIR_RIGHT)
        ? entityPos.x + spriteWidth + GAP
        : entityPos.x - GAP;

    float y = entityPos.y + barrelOffsetY;
    return sf::Vector2f(x, y);
}

void ProjectileManager::angleToVelocity(float angle, int dir, float speed,
    float& outVX, float& outVY)
{
    float rad = angle * 3.14159f / 180.f;
    outVX = (dir == DIR_RIGHT ? 1.f : -1.f) * cosf(rad) * speed;
    outVY = -sinf(rad) * speed;
}

void ProjectileManager::spawnStraight(sf::Vector2f origin, int dir,
    float angle, int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    StraightProjectile* p = new StraightProjectile(this->texMgr, this->audMgr, angle);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 15.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

void ProjectileManager::spawnExplosive(sf::Vector2f origin, int dir,
    float angle, int dmg,
    int blastRadius, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;
    p->blastRadius = blastRadius;

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 8.f, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}

void ProjectileManager::update(float scroll, Level* lvl) {
    (void)lvl;
    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p != nullptr && p->status) {
            p->move(scroll);
        }
    }
}

void ProjectileManager::postEntityUpdate(float scroll, Level* lvl) {
    int i = 0;
    while (i < this->activeCount) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->status) {
            this->removeAt(i);
            continue;
        }

        if (lvl != nullptr) {
            p->checkTileCollision(lvl);
        }

        if (!p->status) {
            this->removeAt(i);
            continue;
        }

        p->checkBounds(scroll);
        if (!p->status) {
            this->removeAt(i);
            continue;
        }

        i++;
    }
}

void ProjectileManager::draw(RenderWindow& window, float scroll) {
    RectangleShape rect(sf::Vector2f(8.f, 6.f));   // CHANGED: was (16.f, 24.f)

    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->getStatus()) continue;

        rect.setPosition(p->position.x - scroll, p->position.y);

        if (p->isExplosive) {
            rect.setFillColor(Color(255, 140, 0));
        }
        else {
            rect.setFillColor(Color(255, 255, 0));
        }

        window.draw(rect);
    }
}

int ProjectileManager::checkEntityCollisions(DamagableEntity** targets,
    int targetCount)
{
    int totalDamage = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();
        bool    hit = false;

        for (int e = 0; e < targetCount && !hit; e++) {
            if (targets[e] == nullptr || !targets[e]->isAlive()) continue;

            IntRect entBox = targets[e]->getBoundingBox();

            bool overlapX = (projBox.left < entBox.left + entBox.width) &&
                (projBox.left + projBox.width > entBox.left);
            bool overlapY = (projBox.top < entBox.top + entBox.height) &&
                (projBox.top + projBox.height > entBox.top);

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

void ProjectileManager::removeAt(int i) {
    delete this->slots[i];
    this->slots[i] = nullptr;
    this->activeCount--;

    if (i < this->activeCount) {
        this->slots[i] = this->slots[this->activeCount];
        this->slots[this->activeCount] = nullptr;
    }
}