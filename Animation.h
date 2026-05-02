#pragma once
#include "Constants.h"

// WHY: Animation class handles sprite frame sequencing for entities
// Each entity has one Animation that cycles through frames
class Animation {
public:
    Texture* texture;
    int frameCount;
    int currentFrame;
    int frameDelay;  // frames per image
    bool loop;
    int srcOffsetX;  // pixel X on spritesheet where first frame starts
    int srcOffsetY;  // pixel Y on spritesheet where frames row starts

    Clock clock;

    Animation();
    ~Animation();

    void update();
    void applyToSprite(Sprite& sprite);
    void reset();
    void setLoop(bool shouldLoop);
    void setFrameCount(int count);
    void setFrameDelay(int delay);
    void setTexture(Texture* tex);
    void setSrcOffset(int x, int y);  // spritesheet origin for first frame
    bool isFinished() const;
};