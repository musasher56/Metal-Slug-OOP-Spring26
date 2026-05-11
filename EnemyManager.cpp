#include "EnemyManager.h"
#include "Enemy.h"
#include "PlayerSoldier.h"
#include "Level.h"
#include "ProjectileManager.h"
#include <cstdio>

EnemyManager::EnemyManager(TextureManager* t, AudioManager* a)
    : activeCount(0), texMgr(t), audMgr(a), pm(nullptr), activeBoss(nullptr)
    , bossDied(false), bossDiedName(nullptr)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        this->slots[i] = nullptr;
        this->deSlots[i] = nullptr;
    }
}

EnemyManager::~EnemyManager() {
    this->clearAll();
}

void EnemyManager::setProjectileManager(ProjectileManager* p) {
    this->pm = p;
}

int EnemyManager::findFreeSlot() {
    if (this->activeCount < MAX_ENEMIES) {
        return this->activeCount;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = this->slots[i];
        if (e == nullptr || (!e->isAlive() && !e->isDying())) {
            return i;
        }
    }

    return -1;
}

void EnemyManager::removeAt(int i) {
    if (this->slots[i] != nullptr) {
        
        if (this->activeBoss != nullptr && this->slots[i] == this->activeBoss) {
            this->bossDiedName = this->activeBoss->getBossName();
            this->bossDied = true;
            this->activeBoss = nullptr;
        }
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

int EnemyManager::spawnRebel(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Enemy* enemy = new RebelSoldier(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setPatrol(x, 80.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

int EnemyManager::spawnBazooka(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Enemy* enemy = new BazookaSoldier(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setPatrol(x, 80.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

int EnemyManager::spawnShielded(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Enemy* enemy = new ShieldedSoldier(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setPatrol(x, 80.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

int EnemyManager::spawnGrenade(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Enemy* enemy = new GrenadeSoldier(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setPatrol(x, 80.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

int EnemyManager::spawnMartian(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Enemy* enemy = new Martian(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setPatrol(x, 100.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

int EnemyManager::spawnParatrooper(float x, float y, float landY) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Paratrooper* enemy = new Paratrooper(this->texMgr, this->audMgr);
    enemy->position = sf::Vector2f(x, y);
    enemy->setLandY(landY);
    enemy->setPatrol(x, 80.f);
    enemy->setProjectileManager(this->pm);
    enemy->updateBoundingBox();

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = enemy;
    this->deSlots[slot] = enemy;
    return slot;
}

void EnemyManager::update(float scrollX, float scrollY, Level* lvl,
    PlayerSoldier* player)
{
    (void)scrollX;
    (void)scrollY;

    int i = 0;
    while (i < this->activeCount) {
        Enemy* e = this->slots[i];
        if (e == nullptr || (!e->isAlive() && !e->isDying()) || !e->getStatus()) {
            this->removeAt(i);
            continue;
        }

        e->updateAI(player, lvl);
        e->update(0.f, lvl);
        e->updateBoundingBox();

        i++;
    }
}

void EnemyManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    for (int i = 0; i < this->activeCount; i++) {
        Enemy* e = this->slots[i];
        if (e == nullptr || !e->getStatus()) continue;

        float sx = e->position.x - scrollX;
        float sy = e->position.y - scrollY;

        
        
        float cullMargin = 200.f;
        if (dynamic_cast<Boss*>(e) != nullptr) {
            cullMargin = 800.f;  
        }

        if (sx < -cullMargin || sx > SCREEN_W + cullMargin ||
            sy < -cullMargin || sy > SCREEN_H + cullMargin) continue;

        e->draw(window, scrollX, scrollY);
    }
}

int EnemyManager::spawnIronokava(float x, float y) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Ironokava* boss = new Ironokava(this->texMgr, this->audMgr);
    boss->position = sf::Vector2f(x, y);
    boss->setPatrol(x, 300.f);
    boss->setProjectileManager(this->pm);
    boss->updateBoundingBox();

    
    this->activeBoss = boss;

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = boss;
    this->deSlots[slot] = boss;
    return slot;
}

int EnemyManager::spawnHairbuster(float x, float y, float cx, float cy) {
    int slot = this->findFreeSlot();
    if (slot < 0) return -1;

    Hairbuster* boss = new Hairbuster(this->texMgr, this->audMgr);
    boss->position = sf::Vector2f(x, y);
    boss->setFlyCenter(cx, cy);  
    boss->setPatrol(cx, 500.f);
    boss->setProjectileManager(this->pm);
    boss->updateBoundingBox();

    
    this->activeBoss = boss;

    if (slot < this->activeCount) {
        if (this->slots[slot] != nullptr) {
            delete this->slots[slot];
        }
    }
    else {
        this->activeCount = slot + 1;
    }

    this->slots[slot] = boss;
    this->deSlots[slot] = boss;
    return slot;
}

DamagableEntity** EnemyManager::getDamagableSlots() {
    return this->deSlots;
}

int EnemyManager::getActiveCount() const {
    return this->activeCount;
}

int EnemyManager::getTotalKills() const {
    return 0;
}

Boss* EnemyManager::getActiveBoss() const {
    return this->activeBoss;
}

bool EnemyManager::hasActiveBoss() const {
    if (this->activeBoss == nullptr)
        return false;
    return this->activeBoss->isAlive() || this->activeBoss->isDying();
}

bool EnemyManager::isBossDead() const {
    if (this->bossDied) return true;
    if (this->activeBoss == nullptr) return false;
    return !this->activeBoss->isAlive() && !this->activeBoss->isDying();
}

bool EnemyManager::wasBossKilled() const {
    return this->bossDied;
}

const char* EnemyManager::getBossDiedName() const {
    return this->bossDiedName;
}

void EnemyManager::resetBossDied() {
    this->bossDied = false;
    this->bossDiedName = nullptr;
}
void EnemyManager::cleanup() {
    int i = 0;
    while (i < this->activeCount) {
        if (this->slots[i] == nullptr || (!this->slots[i]->isAlive() && !this->slots[i]->isDying()) ||
            !this->slots[i]->getStatus()) {
            this->removeAt(i);
        }
        else {
            i++;
        }
    }
}

void EnemyManager::clearAll() {
    for (int i = 0; i < this->activeCount; i++) {
        if (this->slots[i] != nullptr) {
            delete this->slots[i];
            this->slots[i] = nullptr;
            this->deSlots[i] = nullptr;
        }
    }
    this->activeCount = 0;
    this->activeBoss = nullptr;
    this->bossDied = false;
    this->bossDiedName = nullptr;
}