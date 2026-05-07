#pragma once
#include "Constants.h"



class Animation {
public:
    Texture* texture;
    int frameCount;
    int currentFrame;
    int frameDelay;  
    bool loop;
    int srcOffsetX;  
    int srcOffsetY;  

    
    
    
    
    int cropLeft;      
    int cropTop;       
    int displayWidth;  
    int displayHeight; 

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
    void setSrcOffset(int x, int y);  
    void setDisplayCrop(int left, int top, int w, int h);  
    bool isFinished() const;
};