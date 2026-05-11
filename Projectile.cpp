#include "Projectile.h"
#include "Level.h"
#include <cmath>


Projectile::Projectile(TextureManager* texMgr, AudioManager* audMgr)
    : Entity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , damage(1)
    , fromEnemy(false)
    , isExplosive(false)
    , blastRadius(0)
    , projectileClass(0)
{
}

Projectile::~Projectile() {}

void Projectile::setVelocity(float vx, float vy) {
    this->velocityX = vx;
    this->velocityY = vy;
}

void Projectile::update(float scroll, Level* lvl) {
    if (!this->status) return;

    this->move(scroll);

    if (lvl != nullptr) {
        this->checkTileCollision(lvl);
    }

    if (!this->status) return;

    this->checkBounds(scroll, 0.f);
}

void Projectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    sf::RectangleShape dbg(sf::Vector2f(8.f, 6.f));
    dbg.setFillColor(sf::Color(255, 0, 255));
    dbg.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(dbg);
}

void Projectile::checkTileCollision(Level* lvl) {
    if (lvl == nullptr) return;

    const int cell = lvl->getCellSize();
    const int worldOffX = lvl->getWorldOffX();
    const int PROJ_W = 8;
    const int PROJ_H = 8;

    float frontX = (this->velocityX >= 0.f)
        ? this->position.x + PROJ_W
        : this->position.x;

    float frontY = (this->velocityY >= 0.f)
        ? this->position.y + PROJ_H
        : this->position.y;

    int colX = (int)frontX / cell - worldOffX;
    int rowY = (int)frontY / cell;

    int rowForX = (int)(this->position.y + PROJ_H / 2.f) / cell;
    int colForY = (int)(this->position.x + PROJ_W / 2.f) / cell - worldOffX;

    bool hitX = lvl->isSolid(rowForX, colX);
    bool hitY = lvl->isSolid(rowY, colForY);

    if (hitX || hitY) {
        this->onImpact(nullptr, nullptr);
        this->deactivate();
    }
}

void Projectile::checkBounds(float scrollX, float scrollY) {
  
    const float MARGIN = 300.f;

    if (this->position.x + 8.f < scrollX - MARGIN) { this->deactivate(); return; }
    if (this->position.x > scrollX + SCREEN_W + MARGIN) { this->deactivate(); return; }
    if (this->position.y + 8.f < scrollY - MARGIN) { this->deactivate(); return; }
    if (this->position.y > scrollY + SCREEN_H + MARGIN) { this->deactivate(); }
}

IntRect Projectile::getBoundingBox() const {
    return IntRect(
        static_cast<int>(this->position.x),
        static_cast<int>(this->position.y),
        8, 6
    );
}

int  Projectile::getDamage()   const { return this->damage; }
bool Projectile::isFromEnemy() const { return this->fromEnemy; }

void Projectile::onImpact(EnemyManager* em, CharacterManager* cm) {
    (void)em; (void)cm;
}


StraightProjectile::StraightProjectile(TextureManager* texMgr,
    AudioManager* audMgr,
    float           ang)
    : Projectile(texMgr, audMgr)
    , angle(ang)
{
    this->projectileClass = PROJ_STRAIGHT;
    this->status = true;
}

StraightProjectile::~StraightProjectile() {}

void StraightProjectile::move(float /*scroll*/) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

void StraightProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;

    if (this->textureManager->loadTexture("bullet_draw", "resources/Sprites/bullet.png")) {
        sf::Texture& tex = this->textureManager->getTexture("bullet_draw");
        sf::Sprite bulletSprite;
        bulletSprite.setTexture(tex);
        float texW = static_cast<float>(tex.getSize().x);
        float texH = static_cast<float>(tex.getSize().y);
        bulletSprite.setOrigin(texW * 0.5f, texH * 0.5f);
        bulletSprite.setScale(0.2f, 0.2f);
        bulletSprite.setRotation(rot);
        bulletSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(bulletSprite);
    }
    else {
        sf::RectangleShape bullet(sf::Vector2f(10.f, 4.f));
        bullet.setFillColor(sf::Color(255, 240, 40));
        bullet.setOrigin(5.f, 2.f);
        bullet.setRotation(rot);
        bullet.setPosition(this->position.x - scrollX, this->position.y - scrollY);
        window.draw(bullet);
    }
}


BallisticProjectile::BallisticProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : Projectile(texMgr, audMgr)
    , gravity(0.5f)
{
    this->projectileClass = PROJ_BALLISTIC;
}

BallisticProjectile::~BallisticProjectile() {}

void BallisticProjectile::move(float /*scroll*/) {
    this->velocityY += this->gravity;
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;
}

void BallisticProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    sf::RectangleShape shot(sf::Vector2f(9.f, 6.f));
    shot.setFillColor(sf::Color(200, 100, 20));  // dark orange
    shot.setOrigin(4.5f, 3.f);

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;
    shot.setRotation(rot);
    shot.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(shot);
}


ExplosiveProjectile::ExplosiveProjectile(TextureManager* texMgr, AudioManager* audMgr)
    : BallisticProjectile(texMgr, audMgr)
{
    this->isExplosive = true;
    this->blastRadius = 3;
    this->projectileClass = PROJ_EXPLOSIVE;
    this->status = true;
}

ExplosiveProjectile::~ExplosiveProjectile() {}

void ExplosiveProjectile::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) return;

    float rot = std::atan2f(this->velocityY, this->velocityX) * 180.f / 3.14159f;

    if (this->projectileClass == PROJ_BOMB) {
     
        if (this->textureManager->loadTexture("bomb_draw", "resources/Sprites/bomb.png")) {
            sf::Texture& tex = this->textureManager->getTexture("bomb_draw");
            sf::Sprite bombSprite;
            bombSprite.setTexture(tex);
            bombSprite.setTextureRect(sf::IntRect(0, 0, 108, 52));
            bombSprite.setOrigin(54.f, 26.f);
            bombSprite.setScale(0.8f, 0.8f);
            bombSprite.setRotation(rot);
            bombSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(bombSprite);
        }
        else {
            sf::CircleShape bomb(7.f);
            bomb.setFillColor(sf::Color(55, 55, 55));
            bomb.setOutlineColor(sf::Color(120, 120, 120));
            bomb.setOutlineThickness(1.5f);
            bomb.setOrigin(7.f, 7.f);
            bomb.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(bomb);
        }
    }
    else {
    
        if (this->textureManager->loadTexture("grenade_draw", "resources/Sprites/grenade.png")) {
            sf::Texture& tex = this->textureManager->getTexture("grenade_draw");
            sf::Sprite grenadeSprite;
            grenadeSprite.setTexture(tex);
            float texW = static_cast<float>(tex.getSize().x);
            float texH = static_cast<float>(tex.getSize().y);
            grenadeSprite.setOrigin(texW * 0.5f, texH * 0.5f);
            grenadeSprite.setScale(0.08f, 0.08f);
            grenadeSprite.setRotation(rot);
            grenadeSprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(grenadeSprite);
        }
        else {
            sf::RectangleShape rocket(sf::Vector2f(14.f, 7.f));
            rocket.setFillColor(sf::Color(255, 90, 10));
            rocket.setOrigin(7.f, 3.5f);
            rocket.setRotation(rot);
            rocket.setPosition(this->position.x - scrollX, this->position.y - scrollY);
            window.draw(rocket);

            sf::CircleShape nose(2.5f);
            nose.setFillColor(sf::Color(255, 200, 100));
            nose.setOrigin(2.5f, 2.5f);
            float noseOffX = std::cosf(rot * 3.14159f / 180.f) * 7.f;
            float noseOffY = std::sinf(rot * 3.14159f / 180.f) * 7.f;
            nose.setPosition((this->position.x - scrollX) + noseOffX,
                (this->position.y - scrollY) + noseOffY);
            window.draw(nose);
        }
    }
}

void ExplosiveProjectile::onImpact(EnemyManager* em, CharacterManager* cm) {
 
    (void)em; (void)cm;
}


FlameParticle::FlameParticle(TextureManager* texMgr, AudioManager* audMgr,
    float ang, int frames)
    : StraightProjectile(texMgr, audMgr, ang)
    , lifetime(frames)
    , maxLifetime(frames)
{
    this->projectileClass = PROJ_FLAME;
}

FlameParticle::~FlameParticle() {}

void FlameParticle::move(float scroll) {
    StraightProjectile::move(scroll);

    if (--this->lifetime <= 0) {
        this->deactivate();
    }
}

void FlameParticle::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status)
        return;

    float ratio = (float)this->lifetime / (float)this->maxLifetime;

    sf::Uint8 g = static_cast<sf::Uint8>(90.f * ratio); 
    sf::Uint8 alpha = static_cast<sf::Uint8>(200.f * ratio + 55.f);

    sf::RectangleShape halo(sf::Vector2f(12.f, 8.f));
    halo.setFillColor(sf::Color(255, g, 0, static_cast<sf::Uint8>(alpha / 2)));
    halo.setOrigin(6.f, 4.f);
    halo.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(halo);

    sf::RectangleShape core(sf::Vector2f(8.f, 5.f));
    core.setFillColor(sf::Color(255, g + 50 > 255 ? 255 : g + 50, 10, alpha));
    core.setOrigin(4.f, 2.5f);
    core.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(core);
}


LaserBeam::LaserBeam(TextureManager* texMgr, AudioManager* audMgr,
    int dir, int frames)
    : StraightProjectile(texMgr, audMgr, 0.f)
    , lifetime(frames)
    , beamDir(dir)
{
    this->projectileClass = PROJ_BEAM;
    this->isExplosive = false;
}

LaserBeam::~LaserBeam() {}

IntRect LaserBeam::getBoundingBox() const {
    if (this->beamDir == DIR_RIGHT) {
        return IntRect(
            static_cast<int>(this->position.x),
            static_cast<int>(this->position.y) - 4,
            SCREEN_W, 8
        );
    }
    else {
        return IntRect(
            static_cast<int>(this->position.x) - SCREEN_W,
            static_cast<int>(this->position.y) - 4,
            SCREEN_W, 8
        );
    }
}

void LaserBeam::update(float scroll, Level* /*lvl*/) {
    if (!this->status) return;

    this->move(scroll);
    if (this->status) {
        this->checkBounds(scroll, 0.f);
    }
}

void LaserBeam::move(float /*scroll*/) {
    if (--this->lifetime <= 0) {
        this->deactivate();
    }
}

void LaserBeam::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (!this->status) 
        return;
    float ratio = (float)this->lifetime / 5.f;
    if (ratio > 1.f) ratio = 1.f;

    sf::Uint8 alpha = static_cast<sf::Uint8>(180.f * ratio + 75.f);
    sf::Uint8 coreAlpha = static_cast<sf::Uint8>(230.f * ratio + 25.f);

    float beamLen = static_cast<float>(SCREEN_W);

    float startX = (this->beamDir == DIR_RIGHT)
        ? (this->position.x - scrollX)
        : (this->position.x - scrollX - beamLen);
    float y = this->position.y - scrollY;

    sf::RectangleShape glow(sf::Vector2f(beamLen, 8.f));
    glow.setFillColor(sf::Color(0, 220, 255, alpha));
    glow.setOrigin(0.f, 4.f);
    glow.setPosition(startX, y);
    window.draw(glow);

    sf::RectangleShape core(sf::Vector2f(beamLen, 3.f));
    core.setFillColor(sf::Color(180, 255, 255, coreAlpha));
    core.setOrigin(0.f, 1.5f);
    core.setPosition(startX, y);
    window.draw(core);

    float flashX = (this->beamDir == DIR_RIGHT)
        ? this->position.x - scrollX
        : this->position.x - scrollX;

    sf::CircleShape flash(6.f);
    flash.setFillColor(sf::Color(200, 255, 255, coreAlpha));
    flash.setOrigin(6.f, 6.f);
    flash.setPosition(flashX, y);
    window.draw(flash);
}