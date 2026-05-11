#pragma once
#include "DamagableEntity.h"

class Block : public DamagableEntity {
public:
    static const int TOTAL_FRAMES = 15;
    static const int FRAME_X = 16;
    static const int FRAME_Y = 285;
    static const int FRAME_W = 140;
    static const int FRAME_H = 143;

private:
    bool destroying;
    bool indestructible;
    int gridRow;
    int gridCol;
    Level* level;

public:
    Block(TextureManager* texMgr, AudioManager* audMgr,
        float worldX, float worldY, Level* lvl);
    ~Block();

    void update(float scroll, Level* lvl);
    void draw(RenderWindow& window, float scrollX, float scrollY);

    void takeDamage(int amount);
    void onDeath();
    void updateBoundingBox();

    bool isDestroying() const { return this->destroying; }
    void setIndestructible(bool val) { this->indestructible = val; }
    bool isIndestructible() const { return this->indestructible; }
};





class TextureManager;

class MountainBlock {
private:
    sf::Sprite  sprite;
    float       worldX;
    float       worldY;
    bool        active;
public:
    static const int BLOCK_SIZE = 48;

    MountainBlock(TextureManager* texMgr, float wx, float wy);

    void draw(sf::RenderWindow& window, float scrollX, float scrollY);
    float getWorldX() const { return this->worldX; }
    float getWorldY() const { return this->worldY; }
    bool  getActive() const { return this->active; }
};