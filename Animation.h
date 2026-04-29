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
    
    Clock clock;
    
    Animation();
    ~Animation();
    
    void update();
    void applyToSprite(Sprite& sprite);
    void reset();
    void setLoop(bool shouldLoop);
    void setFrameCount(int count);
    void setTexture(Texture* tex);
    bool isFinished() const;
};
