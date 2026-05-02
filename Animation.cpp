#include "Animation.h"

Animation::Animation()
    : texture(nullptr)
    , frameCount(1)
    , currentFrame(0)
    , frameDelay(5)
    , loop(true)
    , srcOffsetX(0)
    , srcOffsetY(0)
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
        // WHY use FRAME_W directly instead of dividing texture width?
        // Dividing total texture width by frameCount only works when frames
        // start at x=0 and are packed with no padding. When a spritesheet has
        // an offset (e.g. block frames start at x=16, y=285) we must use the
        // known frame width and the stored srcOffset instead.
        int frameWidth = (int)this->texture->getSize().x / this->frameCount;
        int frameHeight = (int)this->texture->getSize().y;
        // If a src offset is set, use fixed frame size from offset
        if (this->srcOffsetX != 0 || this->srcOffsetY != 0) {
            // frameWidth derived from offset: total usable width after offsetX / frames
            frameWidth = ((int)this->texture->getSize().x - this->srcOffsetX) / this->frameCount;
            frameHeight = (int)this->texture->getSize().y - this->srcOffsetY;
        }
        IntRect rect(
            this->srcOffsetX + this->currentFrame * frameWidth,
            this->srcOffsetY,
            frameWidth,
            frameHeight
        );
        sprite.setTextureRect(rect);
    }
}

void Animation::setSrcOffset(int x, int y) {
    this->srcOffsetX = x;
    this->srcOffsetY = y;
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