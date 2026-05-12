#include "EnemyVehicleManager.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include <cstdio>

EnemyVehicleManager::EnemyVehicleManager(TextureManager* t, AudioManager* a)
    : activeCount(0), texMgr(t), audMgr(a), pm(nullptr)
{
    for (int i = 0; i < MAX_VEHICLES; i++) {
        this->slots[i] = nullptr;
        this->deSlots[i] = nullptr;
    }
}

EnemyVehicleManager::~EnemyVehicleManager() {
    this->clearAll();
}

void EnemyVehicleManager::setProjectileManager(ProjectileManager* p) {
    this->pm = p;
}

int EnemyVehicleManager::findFreeSlot() {
    if (this->activeCount < MAX_VEHICLES) {
        return this->activeCount;
    }

    for (int i = 0; i < MAX_VEHICLES; i++) {
        EnemyVehicle* v = this->slots[i];
        if (v == nullptr || (!v->isAlive() && v->isDestroyed())) {
            return i;
        }
    }

    return -1;
}

void EnemyVehicleManager::removeAt(int i) {
    if (this->slots[i] != nullptr) {
        delete this->slots[i];
        this->slots[i] = nullptr;
        this->deSlots[i] = nullptr;
    }
    this->activeCount--;

    if (i < this->activeCount) {
        this->slots[i] = this->slots[this->activeCount];
        this->deSlots[i] = this->deSlots[this->activeCount];
        this->slots[this->activeCount] = nullptr;
        this->deSlots[this->activeCount] = nullptr;
    }
}

int EnemyVehicleManager::spawnFlyingTara(float x, float y, int dir) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    FlyingTara* tara = new FlyingTara(this->texMgr, this->audMgr);
    tara->position = sf::Vector2f(x, y);
    tara->setFlyDirection(dir);
    tara->setProjectileManager(this->pm);
    tara->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = tara;
    this->deSlots[slot] = tara;
    return slot;
}

int EnemyVehicleManager::spawnSubmarine(float x, float y, int dir) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Submarine* sub = new Submarine(this->texMgr, this->audMgr);
    sub->position = sf::Vector2f(x, y);
    sub->setSwimDirection(dir);
    sub->setProjectileManager(this->pm);
    sub->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = sub;
    this->deSlots[slot] = sub;
    return slot;
}

void EnemyVehicleManager::update(float scroll, float scrollY, Level* lvl,
    PlayerSoldier* player, ProjectileManager* projMgr)
{
    (void)scrollY;

    int i = 0;
    while (i < this->activeCount) {
        EnemyVehicle* v = this->slots[i];
        if (v == nullptr || !v->getStatus() || (v->isDestroyed() && !v->isAlive())) {
            this->removeAt(i);
            continue;
        }

        v->update(player, projMgr, scroll, lvl);
        v->updateBoundingBox();

        i++;
    }
}

void EnemyVehicleManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    for (int i = 0; i < this->activeCount; i++) {
        EnemyVehicle* v = this->slots[i];
        if (v == nullptr || !v->getStatus())
            continue;

        float sx = v->position.x - scrollX;
        float sy = v->position.y - scrollY;

        if (sx < -500.f || sx >(float)SCREEN_W + 500.f ||
            sy < -500.f || sy >(float)SCREEN_H + 500.f)
            continue;

        v->draw(window, scrollX, scrollY);
    }
}

DamagableEntity** EnemyVehicleManager::getDamagableSlots() {
    return this->deSlots;
}

int EnemyVehicleManager::getActiveCount() const {
    return this->activeCount;
}

void EnemyVehicleManager::cleanup() {
    int i = 0;
    while (i < this->activeCount) {
        EnemyVehicle* v = this->slots[i];
        if (v == nullptr || !v->getStatus() ||
            (v->isDestroyed() && !v->isAlive())) {
            this->removeAt(i);
        }
        else {
            i++;
        }
    }
}

void EnemyVehicleManager::clearAll() {
    for (int i = 0; i < this->activeCount; i++) {
        if (this->slots[i] != nullptr) {
            delete this->slots[i];
            this->slots[i] = nullptr;
            this->deSlots[i] = nullptr;
        }
    }
    this->activeCount = 0;
}