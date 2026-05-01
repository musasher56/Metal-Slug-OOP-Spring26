#include "Animation.h"

Animation::Animation()
    : texture(nullptr)
    , frameCount(1)
    , currentFrame(0)
    , frameDelay(5)
    , loop(true)
{}

Animation::~Animation() {
    this->texture = nullptr;
}

void Animation::update() {
    if (this->clock.getElapsedTime().asMilliseconds() >= (unsigned int)(this->frameDelay * 16)) {
        this->currentFrame++;
        if (this->currentFrame >= this->frameCount) {
            if (this->loop) {
                this->currentFrame = 0;
            } else {
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
        int frameWidth = this->texture->getSize().x / this->frameCount;
        IntRect rect(
            this->currentFrame * frameWidth, 0,
            frameWidth, (int)this->texture->getSize().y
        );
        sprite.setTextureRect(rect);
    }
}

void Animation::reset() {
    this->currentFrame = 0;
    this->clock.restart();
}

void Animation::setLoop(bool shouldLoop)    { this->loop = shouldLoop; }
void Animation::setFrameCount(int count)    { this->frameCount = count; }
void Animation::setTexture(Texture* tex)    { this->texture = tex; }
bool Animation::isFinished() const {
    return !this->loop && this->currentFrame >= this->frameCount - 1;
}