#pragma once
#include "DamagableEntity.h"

class Block : public DamagableEntity {
public:
    static const int TOTAL_FRAMES = 15;
    static const int FRAME_X = 16;   // spritesheet X origin
    static const int FRAME_Y = 285;  // spritesheet Y origin
    static const int FRAME_W = 140;
    static const int FRAME_H = 143;

private:
    bool destroying;
    int gridRow;
    int gridCol;
    Level* level;

public:
    Block(TextureManager* texMgr, AudioManager* audMgr,
        float worldX, float worldY, Level* lvl);
    ~Block();

    void update(float scroll, Level* lvl);
    void draw(RenderWindow& window, float scroll);

    void takeDamage(int amount);
    void onDeath();
    void updateBoundingBox();

    bool isDestroying() const { return this->destroying; }
};