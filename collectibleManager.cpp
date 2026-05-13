#include "CollectibleManager.h"
#include "TextureManager.h"
#include "PlayerSoldier.h"
#include <cstdlib>
#include <cmath>

CollectibleManager::CollectibleManager(TextureManager* texMgr)
    : count(0), texMgr(texMgr), foodTexLoaded(false), crateTexLoaded(false)
    , pickupRadius(50.f)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        this->items[i].active = false;
        this->items[i].type = TYPE_FOOD;
        this->items[i].bobTimer = 0.f;
    }
    this->loadTextures();
}

CollectibleManager::~CollectibleManager() {
    this->clearAll();
}

void CollectibleManager::loadTextures() {
    if (this->texMgr != nullptr) {
            this->foodTex.loadFromFile("resources/Sprites/food.png");
            this->foodTexLoaded = true;
            this->crateTex.loadFromFile("resources/Sprites/supplycrate.png");
            this->crateTexLoaded = true;
    }
}

int CollectibleManager::findFreeSlot() {
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!this->items[i].active) {
            return i;
        }
    }
    return -1;
}

void CollectibleManager::spawnCollectibles(float levelWidth, float surfaceY, int levelIndex, bool isBossLevel) {
    this->clearAll();

    int numFood = 3 + (std::rand() % 3);
    int numSupply = 2 + (std::rand() % 2);

    if (isBossLevel) {
        numFood = 2;
        numSupply = 1;
    }

    float spacing = levelWidth / (float)(numFood + numSupply + 2);
    float startOffset = spacing;

    for (int i = 0; i < numFood; i++) {
        int slot = this->findFreeSlot();
        if (slot < 0) 
            break;

        float x = startOffset + (float)(i + (i * 2)) * spacing + ((float)(std::rand() % 200) - 100.f);
        float y = surfaceY - 80.f - ((float)(std::rand() % 60));

        this->items[slot].position = sf::Vector2f(x, y);
        this->items[slot].type = TYPE_FOOD;
        this->items[slot].active = true;
        this->items[slot].bobTimer = (float)(std::rand() % 100) * 0.1f;

        if (this->foodTexLoaded) {
            this->items[slot].sprite.setTexture(this->foodTex);
            float texW = (float)this->foodTex.getSize().x;
            float texH = (float)this->foodTex.getSize().y;
            float scale = 40.f / texH;
            if (scale > 2.f) 
                scale = 2.f;
            if (scale < 0.3f)
                scale = 0.3f;
            this->items[slot].sprite.setScale(scale, scale);
            this->items[slot].sprite.setOrigin(texW * 0.5f, texH * 0.5f);
        }

        this->count++;
    }

    for (int i = 0; i < numSupply; i++) {
        int slot = this->findFreeSlot();
        if (slot < 0) break;

        float x = startOffset + (float)(numFood + i + (i * 2) + 1) * spacing + ((float)(std::rand() % 200) - 100.f);
        float y = surfaceY - 80.f - ((float)(std::rand() % 60));

        this->items[slot].position = sf::Vector2f(x, y);
        this->items[slot].type = TYPE_SUPPLY;
        this->items[slot].active = true;
        this->items[slot].bobTimer = (float)(std::rand() % 100) * 0.1f;

        if (this->crateTexLoaded) {
            this->items[slot].sprite.setTexture(this->crateTex);
            float texW = (float)this->crateTex.getSize().x;
            float texH = (float)this->crateTex.getSize().y;
            float scale = 45.f / texH;
            if (scale > 2.f) scale = 2.f;
            if (scale < 0.3f) scale = 0.3f;
            this->items[slot].sprite.setScale(scale, scale);
            this->items[slot].sprite.setOrigin(texW * 0.5f, texH * 0.5f);
        }

        this->count++;
    }
}

void CollectibleManager::update(float dt, PlayerSoldier* player) {
    (void)dt;

    if (player == nullptr)
        return;

    sf::Vector2f playerPos = player->getPosition();

    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!this->items[i].active)
            continue;

        this->items[i].bobTimer += 0.04f;
        float bobY = sinf(this->items[i].bobTimer) * 5.f;

        this->items[i].sprite.setPosition(
            this->items[i].position.x,
            this->items[i].position.y + bobY
        );

        float dx = playerPos.x - this->items[i].position.x;
        float dy = playerPos.y - this->items[i].position.y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < this->pickupRadius) {
            if (this->items[i].type == TYPE_FOOD) {
                int currentHP = player->getCurrentHP();
                int maxHP = player->getMaxHealth();
                if (currentHP < maxHP) {
                    int healAmount = 1;
                    int newHP = currentHP + healAmount;
                    if (newHP > maxHP) {
                        newHP = maxHP;
                    }
                    player->healBy(healAmount);
                }
            }
            else if (this->items[i].type == TYPE_SUPPLY) {
                player->addGrenades(5);
            }

            this->items[i].active = false;
            this->count--;
        }
    }
}

void CollectibleManager::draw(sf::RenderWindow& window, float scrollX, float scrollY) {
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!this->items[i].active)
            continue;

        float sx = this->items[i].sprite.getPosition().x - scrollX;
        float sy = this->items[i].sprite.getPosition().y - scrollY;

        if (sx < -100.f || sx > SCREEN_W + 100.f ||
            sy < -100.f || sy > SCREEN_H + 100.f) {
            continue;
        }

        this->items[i].sprite.setPosition(sx, sy);
        window.draw(this->items[i].sprite);
    }
}

void CollectibleManager::clearAll() {
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        this->items[i].active = false;
    }
    this->count = 0;
}
