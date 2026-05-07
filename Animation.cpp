#include "Animation.h"

Animation::Animation()
    : texture(nullptr)
    , frameCount(1)
    , currentFrame(0)
    , frameDelay(5)
    , loop(true)
    , srcOffsetX(0)
    , srcOffsetY(0)
    , cropLeft(0)
    , cropTop(0)
    , displayWidth(0)
    , displayHeight(0)
{
}

Animation::~Animation() {
    this->texture = nullptr;
}

void Animation::update() {
    if (this->clock.getElapsedTime().asMilliseconds() >= (unsigned int)(this->frameDelay * 16)) {
        this->currentFrame++;
        if (this->currentFrame >= this->frameCount) {
            if (this->loop) {
                this->currentFrame = 0;
            }
            else {
                this->currentFrame = this->frameCount - 1;
            }
        }
        this->clock.restart();
    }
}

void Animation::applyToSprite(Sprite& sprite) {
    if (this->texture == nullptr) return;
    sprite.setTexture(*this->texture);
    if (this->frameCount > 1 && this->texture->getSize().x > 0) {
        
        
        
        
        
        int frameWidth = (int)this->texture->getSize().x / this->frameCount;
        int frameHeight = (int)this->texture->getSize().y;
        
        if (this->srcOffsetX != 0 || this->srcOffsetY != 0) {
            
            frameWidth = ((int)this->texture->getSize().x - this->srcOffsetX) / this->frameCount;
            frameHeight = (int)this->texture->getSize().y - this->srcOffsetY;
        }

        
        
        
        int rectX = this->srcOffsetX + this->currentFrame * frameWidth + this->cropLeft;
        int rectY = this->srcOffsetY + this->cropTop;
        int rectW = (this->displayWidth > 0) ? this->displayWidth : (frameWidth - this->cropLeft);
        int rectH = (this->displayHeight > 0) ? this->displayHeight : (frameHeight - this->cropTop);

        IntRect rect(rectX, rectY, rectW, rectH);
        sprite.setTextureRect(rect);
    }
}

void Animation::setSrcOffset(int x, int y) {
    this->srcOffsetX = x;
    this->srcOffsetY = y;
}

void Animation::setDisplayCrop(int left, int top, int w, int h) {
    this->cropLeft = left;
    this->cropTop = top;
    this->displayWidth = w;
    this->displayHeight = h;
}

void Animation::reset() {
    this->currentFrame = 0;
    this->clock.restart();
}

void Animation::setLoop(bool shouldLoop) { this->loop = shouldLoop; }
void Animation::setFrameCount(int count) { this->frameCount = count; }
void Animation::setFrameDelay(int delay) { this->frameDelay = delay; }
void Animation::setTexture(Texture* tex) { this->texture = tex; }
bool Animation::isFinished() const {
    return !this->loop && this->currentFrame >= this->frameCount - 1;
}