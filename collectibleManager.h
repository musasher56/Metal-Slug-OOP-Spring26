#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>

class TextureManager;
class PlayerSoldier;

struct CollectibleItem {
    sf::Vector2f position;
    int type;
    bool active;
    sf::Sprite sprite;
    float bobTimer;
};

class CollectibleManager {
public:
    static const int MAX_COLLECTIBLES = 32;
    static const int TYPE_FOOD = 0;
    static const int TYPE_SUPPLY = 1;

    CollectibleManager(TextureManager* texMgr);
    ~CollectibleManager();

    void spawnCollectibles(float levelWidth, float surfaceY, int levelIndex, bool isBossLevel);
    void update(float dt, PlayerSoldier* player);
    void draw(sf::RenderWindow& window, float scrollX, float scrollY);
    void clearAll();

private:
    CollectibleItem items[MAX_COLLECTIBLES];
    int count;
    TextureManager* texMgr;
    sf::Texture foodTex;
    sf::Texture crateTex;
    bool foodTexLoaded;
    bool crateTexLoaded;
    float pickupRadius;

    void loadTextures();
    int findFreeSlot();
};
