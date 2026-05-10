#include "ProjectileManager.h"
#include "Level.h"
#include <cmath>





ProjectileManager::ProjectileManager(TextureManager* t, AudioManager* a)
    : activeCount(0)
    , texMgr(t)
    , audMgr(a)
    , blastCount(0)
{
    for (int i = 0; i < MAX_PROJ; i++) this->slots[i] = nullptr;

    
    
    
    
    this->texMgr->loadTexture("bullet", "resources/Sprites/bullet.png");
    this->texMgr->loadTexture("bullet_draw", "resources/Sprites/bullet.png");
    this->texMgr->loadTexture("grenade_draw", "resources/Sprites/grenade.png");
    this->texMgr->loadTexture("bomb_draw", "resources/Sprites/bomb.png");

    
    
    
    
    
    if (!this->texMgr->loadTexture("blast", "resources/Sprites/blast.png")) {
        this->texMgr->makeColorTexture("blast", sf::Color(255, 140, 0));  
    }
    for (int i = 0; i < MAX_BLASTS; i++) {
        this->blasts[i].active = false;
        this->blasts[i].anim.setTexture(&this->texMgr->getTexture("blast"));
        this->blasts[i].anim.setFrameCount(3);
        this->blasts[i].anim.setFrameDelay(6);
        this->blasts[i].anim.setFrameRect(0, 75, 339, 308, 278);
        this->blasts[i].anim.setFrameRect(1, 463, 353, 457, 287);
        this->blasts[i].anim.setFrameRect(2, 984, 304, 505, 400);
        this->blasts[i].anim.setLoop(false);
    }
}

ProjectileManager::~ProjectileManager() {
    this->clearAll();
}

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
    int          dir,
    float        spriteWidth,
    float        barrelOffsetY)
{
    const float GAP = 4.f;
    float x = (dir == DIR_RIGHT)
        ? entityPos.x + spriteWidth + GAP
        : entityPos.x - GAP;
    return sf::Vector2f(x, entityPos.y + barrelOffsetY);
}

void ProjectileManager::angleToVelocity(float angle, int dir, float speed,
    float& outVX, float& outVY)
{
    
    
    float rad = angle * 3.14159f / 180.f;
    outVX = (dir == DIR_RIGHT ? 1.f : -1.f) * std::cosf(rad) * speed;
    outVY = -std::sinf(rad) * speed;
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

void ProjectileManager::spawnBomb(sf::Vector2f origin, int dir,
    float angle, int dmg, int blastRadius,
    bool fromEnemy, float speed)
{
    if (this->activeCount >= MAX_PROJ) return;

    ExplosiveProjectile* p = new ExplosiveProjectile(this->texMgr, this->audMgr);
    p->position = origin;
    p->fromEnemy = fromEnemy;
    p->damage = dmg;
    p->blastRadius = blastRadius;
    p->projectileClass = PROJ_BOMB;   

    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, speed, vx, vy);
    p->setVelocity(vx, vy);

    this->slots[this->activeCount++] = p;
}










void ProjectileManager::spawnFlame(sf::Vector2f origin, int dir,
    float angle, int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    
    
    
    FlameParticle* fp = new FlameParticle(this->texMgr, this->audMgr, angle, 20);
    fp->position = origin;
    fp->fromEnemy = fromEnemy;
    fp->damage = dmg;

    
    
    float vx = 0.f, vy = 0.f;
    ProjectileManager::angleToVelocity(angle, dir, 6.f, vx, vy);
    fp->setVelocity(vx, vy);

    this->slots[this->activeCount++] = fp;
}









void ProjectileManager::spawnLaser(sf::Vector2f origin, int dir,
    int dmg, bool fromEnemy)
{
    if (this->activeCount >= MAX_PROJ) return;

    LaserBeam* lb = new LaserBeam(this->texMgr, this->audMgr, dir, 5);
    lb->position = origin;
    lb->fromEnemy = fromEnemy;
    lb->damage = dmg;
    

    this->slots[this->activeCount++] = lb;
}

void ProjectileManager::spawnBlast(float x, float y) {
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
            
            float  impactX = p->position.x;
            float  impactY = p->position.y;
            bool   wasExplosive = p->isExplosive;

            
            
            
            
            
            p->checkTileCollision(lvl);

            if (!p->status && wasExplosive) {
                this->spawnBlast(impactX, impactY);
            }
        }

        if (!p->status) { this->removeAt(i); continue; }

        
        p->checkBounds(scrollX, scrollY);
        if (!p->status) { this->removeAt(i); continue; }

        i++;
    }

    
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
    for (int i = 0; i < this->activeCount; i++) {
        Projectile* p = this->slots[i];
        if (p != nullptr && p->getStatus()) {
            
            p->draw(window, scrollX, scrollY);
        }
    }

    
    
    
    for (int b = 0; b < MAX_BLASTS; b++) {
        if (!this->blasts[b].active) continue;

        BlastEffect& blast = this->blasts[b];
        Sprite blastSprite;
        blast.anim.applyToSprite(blastSprite);

        IntRect rect = blastSprite.getTextureRect();
        blastSprite.setOrigin(
            static_cast<float>(rect.width) * 0.5f,
            static_cast<float>(rect.height) * 0.5f);
        blastSprite.setScale(0.8f, 0.8f);
        blastSprite.setPosition(blast.x - scrollX, blast.y - 85.f - scrollY);
        window.draw(blastSprite);
    }
}





int ProjectileManager::checkEntityCollisions(DamagableEntity** targets, int targetCount)
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
                if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);
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

int ProjectileManager::checkPlayerBulletHits(DamagableEntity** targets, int targetCount)
{
    int hits = 0;

    for (int p = 0; p < this->activeCount; ) {
        Projectile* proj = this->slots[p];
        if (proj == nullptr || !proj->getStatus()) { p++; continue; }
        if (proj->isFromEnemy()) { p++; continue; }

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
                if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);

                
                
                
                
                
                int bulletDir = (proj->velocityX >= 0.f) ? 1 : -1;
                targets[e]->takeDamageFrom(proj->getDamage(), bulletDir);
                hits++;

                
                
                if (proj->projectileClass != PROJ_BEAM) {
                    proj->deactivate();
                    hit = true;
                }
            }
        }

        if (!proj->getStatus()) this->removeAt(p);
        else                    p++;
    }

    return hits;
}

bool ProjectileManager::checkEnemyBulletHitPlayer(DamagableEntity* player)
{
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
            if (proj->isExplosive) this->spawnBlast(proj->position.x, proj->position.y);
            player->takeDamage(proj->getDamage());
            proj->deactivate();
            if (!proj->getStatus()) this->removeAt(p);
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