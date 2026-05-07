#pragma once
#include "Constants.h"



class Animation {
public:
    static const int MAX_ANIM_FRAMES = 32;

    Texture* texture;
    int frameCount;
    int currentFrame;
    int frameDelay;
    bool loop;
    bool useExplicitFrames;
    IntRect frames[MAX_ANIM_FRAMES];

    int srcOffsetX;
    int srcOffsetY;
    int cropLeft;
    int cropTop;
    int displayWidth;
    int displayHeight;
    int frameSizeW;
    int frameSizeH;

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
    void setFrameRect(int index, int x, int y, int w, int h);
    void setSrcOffset(int x, int y);
    void setDisplayCrop(int left, int top, int w, int h);
    void setFrameSize(int w, int h);
    bool isFinished() const;
};
