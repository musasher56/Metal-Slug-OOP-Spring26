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

    // Display crop — trims empty space from each frame for tighter visuals
    // Example: if each frame is 48px wide but the character art only
    // occupies pixels 10..38, set cropLeft=10, displayWidth=28.
    // Set displayWidth/displayHeight to 0 to use the full frame size.
    int cropLeft;      // pixels to skip from left edge of each frame
    int cropTop;       // pixels to skip from top edge of each frame
    int displayWidth;  // visible width within frame (0 = full frameWidth)
    int displayHeight; // visible height within frame (0 = full frameHeight)

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
    void setDisplayCrop(int left, int top, int w, int h);  // trim empty space
    bool isFinished() const;
};