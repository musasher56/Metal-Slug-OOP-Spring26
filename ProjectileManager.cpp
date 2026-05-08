#include "ProjectileManager.h"
#include "Level.h"
#include <cmath>

ProjectileManager::ProjectileManager(TextureManager* t, AudioManager* a)
    : activeCount(0), texMgr(t), audMgr(a), blastCount(0)
{
    for (int i = 0; i < MAX_PROJ; i++) this->slots[i] = nullptr;
    this->texMgr->loadTexture("bullet", "resources/Sprites/bullet.png");

    // Load blast texture and init blast pool
    this->texMgr->loadTexture("blast", "resources/Sprites/blast.png");
    for (int i = 0; i < MAX_BLASTS; i++) {
        this->blasts[i].active = false;
        this->blasts[i].anim.setTexture(
            &this->texMgr->getTexture("blast"));
        this->blasts[i].anim.setFrameCount(3);
        this->blasts[i].anim.setFrameDelay(6);
        this->blasts[i].anim.setFrameRect(0, 75, 339, 278, 308);
        this->blasts[i].anim.setFrameRect(1, 463, 353, 457, 287);
        this->blasts[i].anim.setFrameRect(2, 984, 304, 505, 400);
        this->blasts[i].anim.setLoop(false);
    }
}

ProjectileManager::~ProjectileManager() { this->clearAll(); }

void ProjectileManager::clearAll() {
    for (int i = 0; i < this->activeCount; i++) {
        delete this->slots[i];
        this->slots[i] = nullptr;
    }
    this->activeCount = 0;
    for (int i = 0; i < MAX_BLASTS; i++) {
        this->blasts[i].active = false;
    }
    this->blastCount = 0;
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


void ProjectileManager::spawnBomb(sf::Vector2f origin, int dir,
    float angle, int dmg, int blastRadius, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;
    p->blastRadius = blastRadius;
    p->projectileClass = PROJ_BOMB;   // <-- marks it as a bomb, not grenade

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 8.f, vx, vy);
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

void ProjectileManager::spawnBlast(float x, float y) {
    // Find a free slot in the blast pool
    for (int i = 0; i < MAX_BLASTS; i++) {
        if (!this->blasts[i].active) {
            this->blasts[i].x = x;
            this->blasts[i].y = y;
            this->blasts[i].active = true;
            this->blasts[i].anim.reset();
            return;
        }
    }
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

void ProjectileManager::postEntityUpdate(float scrollX, float scrollY, Level* lvl) {
    int i = 0;
    while (i < this->activeCount) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->status) {
            this->removeAt(i);
            continue;
        }

        if (lvl != nullptr) {
            // Save position before collision check — if the projectile
            // dies on impact, we know where to spawn the blast effect
            float impactX = p->position.x;
            float impactY = p->position.y;
            bool wasExplosive = p->isExplosive;

            p->checkTileCollision(lvl);

            if (!p->status && wasExplosive) {
                // Grenade hit something — spawn blast at impact point
                this->spawnBlast(impactX, impactY);
            }
        }

        if (!p->status) {
            this->removeAt(i);
            continue;
        }

        p->checkBounds(scrollX, scrollY);
        if (!p->status) {
            this->removeAt(i);
            continue;
        }

        i++;
    }

    // Update blast animations — remove finished ones
    for (int b = 0; b < MAX_BLASTS; b++) {
        if (this->blasts[b].active) {
            this->blasts[b].anim.update();
            if (this->blasts[b].anim.isFinished()) {
                this->blasts[b].active = false;
            }
        }
    }
}

void ProjectileManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    Texture& bulletTex = this->texMgr->getTexture("bullet");
    float texW = static_cast<float>(bulletTex.getSize().x);
    float texH = static_cast<float>(bulletTex.getSize().y);

    Sprite bulletSprite;
    bulletSprite.setTexture(bulletTex);
    bulletSprite.setScale(0.2f, 0.2f);
    bulletSprite.setOrigin(texW * 0.5f, texH * 0.5f);

    // Grenade sprite for explosive projectiles
    bool hasGrenade = this->texMgr->loadTexture("grenade_draw", "resources/Sprites/grenade.png");
    Sprite grenadeSprite;
    float grenadeTexW = 16.f;
    float grenadeTexH = 16.f;
    if (hasGrenade) {
        Texture& gt = this->texMgr->getTexture("grenade_draw");
        grenadeSprite.setTexture(gt);
        grenadeTexW = static_cast<float>(gt.getSize().x);
        grenadeTexH = static_cast<float>(gt.getSize().y);
        grenadeSprite.setOrigin(grenadeTexW * 0.5f, grenadeTexH * 0.5f);
        grenadeSprite.setScale(0.08f, 0.08f);
    }

    // Bomb sprite for bomb projectiles
    bool hasBomb = this->texMgr->loadTexture("bomb_draw", "resources/Sprites/bomb.png");
    Sprite bombSprite;
    float bombTexW = 16.f;
    float bombTexH = 16.f;
    if (hasBomb) {
        Texture& bt = this->texMgr->getTexture("bomb_draw");
        bombSprite.setTexture(bt);
        bombSprite.setTextureRect(IntRect(0, 0, 108, 52));
        bombTexW = 108.f;
        bombTexH = 52.f;
        bombSprite.setOrigin(bombTexW * 0.5f, bombTexH * 0.5f);
        bombSprite.setScale(0.8f, 0.8f);
    }

    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p == nullptr || !p->getStatus()) continue;

        if (p->isExplosive) {
            if (p->projectileClass == PROJ_BOMB) {
                // Draw bomb sprite
                if (hasBomb) {
                    float rot = atan2f(p->velocityY, p->velocityX) * 180.f / 3.14159f;
                    bombSprite.setRotation(rot);
                    bombSprite.setPosition(
                        p->position.x - scrollX,
                        p->position.y - scrollY);
                    window.draw(bombSprite);
                }
                else {
                    RectangleShape bombRect(sf::Vector2f(20.f, 12.f));
                    bombRect.setPosition(p->position.x - scrollX,
                        p->position.y - scrollY);
                    bombRect.setFillColor(Color(80, 80, 80));
                    window.draw(bombRect);
                }
            }
            else {
                // Draw grenade sprite
                if (hasGrenade) {
                    float rot = atan2f(p->velocityY, p->velocityX) * 180.f / 3.14159f;
                    grenadeSprite.setRotation(rot);
                    grenadeSprite.setPosition(
                        p->position.x - scrollX,
                        p->position.y - scrollY);
                    window.draw(grenadeSprite);
                }
                else {
                    RectangleShape explosiveRect(sf::Vector2f(10.f, 8.f));
                    explosiveRect.setPosition(p->position.x - scrollX,
                        p->position.y - scrollY);
                    explosiveRect.setFillColor(Color(255, 140, 0));
                    window.draw(explosiveRect);
                }
            }
        }
        else {
            float rot = atan2f(p->velocityY, p->velocityX) * 180.f / 3.14159f;
            bulletSprite.setRotation(rot);
            bulletSprite.setPosition(
                p->position.x + 4.f - scrollX,
                p->position.y + 3.f - scrollY);
            window.draw(bulletSprite);
        }
    }

    // Draw blast effects
    for (int b = 0; b < MAX_BLASTS; b++) {
        if (!this->blasts[b].active) continue;

        BlastEffect* blast = &this->blasts[b];
        Sprite blastSprite;
        blast->anim.applyToSprite(blastSprite);

        // Center the blast frame on the impact point
        IntRect rect = blastSprite.getTextureRect();
        float blastScale = 0.8f;
        blastSprite.setOrigin(
            static_cast<float>(rect.width) * 0.5f,
            static_cast<float>(rect.height) * 0.5f);
        blastSprite.setScale(blastScale, blastScale);
        blastSprite.setPosition(
            blast->x - scrollX,
            blast->y - 85.f - scrollY);
        window.draw(blastSprite);
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
                // Spawn blast if explosive hits a block
                if (proj->isExplosive) {
                    this->spawnBlast(proj->position.x, proj->position.y);
                }
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

int ProjectileManager::checkPlayerBulletHits(DamagableEntity** targets,
    int targetCount)
{
    int hits = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }

        if (proj->isFromEnemy()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();
        bool hit = false;

        for (int e = 0; e < targetCount && !hit; e++) {
            if (targets[e] == nullptr || !targets[e]->isAlive()) continue;

            IntRect entBox = targets[e]->getBoundingBox();

            bool overlapX = (projBox.left < entBox.left + entBox.width) &&
                (projBox.left + projBox.width > entBox.left);
            bool overlapY = (projBox.top < entBox.top + entBox.height) &&
                (projBox.top + projBox.height > entBox.top);

            if (overlapX && overlapY) {
                // Spawn blast if explosive (grenade) hits an enemy
                if (proj->isExplosive) {
                    this->spawnBlast(proj->position.x, proj->position.y);
                }
                int bulletDir = (proj->velocityX > 0.f) ? 1 : -1;
                targets[e]->takeDamageFrom(proj->getDamage(), bulletDir);
                hits++;
                proj->deactivate();
                hit = true;
            }
        }

        if (!proj->getStatus()) this->removeAt(p);
        else                    p++;
    }

    return hits;
}

bool ProjectileManager::checkEnemyBulletHitPlayer(DamagableEntity* player) {
    if (player == nullptr || !player->isAlive()) return false;

    IntRect playerBox = player->getBoundingBox();

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }

        if (!proj->isFromEnemy()) { p++; continue; }

        IntRect projBox = proj->getBoundingBox();

        bool overlapX = (projBox.left < playerBox.left + playerBox.width) &&
            (projBox.left + projBox.width > playerBox.left);
        bool overlapY = (projBox.top < playerBox.top + playerBox.height) &&
            (projBox.top + projBox.height > playerBox.top);

        if (overlapX && overlapY) {
            // Spawn blast if enemy grenade hits player
            if (proj->isExplosive) {
                this->spawnBlast(proj->position.x, proj->position.y);
            }
            player->takeDamage(proj->getDamage());
            proj->deactivate();
            if (!proj->getStatus()) {
                this->removeAt(p);
            }
            return true;
        }

        p++;
    }

    return false;
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