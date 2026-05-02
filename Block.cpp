#include "Block.h"
#include "Level.h"

Block::Block(TextureManager* texMgr, AudioManager* audMgr,
    float worldX, float worldY, Level* lvl)
    : DamagableEntity(texMgr, audMgr)
    , destroying(false)
    , level(lvl)
{
    // Use the SAME cell size as the level grid (48px)
    // so blocks align perfectly with player gravity collision
    int cellSize = 48;
    if (lvl != nullptr) cellSize = lvl->getCellSize();

    // Snap to nearest grid cell
    this->gridCol = static_cast<int>(worldX / cellSize);
    this->gridRow = static_cast<int>(worldY / cellSize);

    // Clamp to level bounds
    if (lvl != nullptr) {
        if (this->gridCol < 0) this->gridCol = 0;
        if (this->gridCol >= lvl->getWidth()) this->gridCol = lvl->getWidth() - 1;
        if (this->gridRow < 0) this->gridRow = 0;
        if (this->gridRow >= lvl->getHeight()) this->gridRow = lvl->getHeight() - 1;
    }

    // Position = exact grid position
    this->position = sf::Vector2f(
        static_cast<float>(this->gridCol * cellSize),
        static_cast<float>(this->gridRow * cellSize)
    );

    this->health = 1;
    this->maxHealth = 1;
    this->scoreValue = 10;

    // Mark grid cell solid so player gravity detects it as floor
    if (lvl != nullptr) {
        lvl->setSolid(this->gridRow, this->gridCol, true);
    }

    // Load block sprite
    Texture& tex = texMgr->getTexture("resources/sprites/blocks/block.png");
    this->animation.setTexture(&tex);
    this->animation.setFrameCount(TOTAL_FRAMES);
    this->animation.setLoop(false);
    this->animation.setFrameDelay(4);
    this->sprite.setTexture(tex);

    // Scale sprite to fit exactly one cell
    float scaleX = static_cast<float>(cellSize) / static_cast<float>(FRAME_W);
    float scaleY = static_cast<float>(cellSize) / static_cast<float>(FRAME_H);
    this->sprite.setScale(scaleX, scaleY);

    this->sprite.setTextureRect(IntRect(0, 0, FRAME_W, FRAME_H));
    this->boundingBox = IntRect(0, 0, cellSize, cellSize);
}

Block::~Block() {
    // Safety: clear grid if block is still active when destroyed
    if (this->status && this->level != nullptr) {
        this->level->setSolid(this->gridRow, this->gridCol, false);
    }
}

void Block::takeDamage(int amount) {
    if (amount < 0) return;
    if (this->destroying) return;
    if (!this->status) return;

    this->health -= amount;
    if (this->health < 0) this->health = 0;

    if (this->health <= 0) {
        this->destroying = true;
        this->animation.setFrameCount(TOTAL_FRAMES);
        this->animation.setLoop(false);
        this->animation.reset();

        // Clear grid IMMEDIATELY so player falls through
        // while destruction animation plays
        if (this->level != nullptr) {
            this->level->setSolid(this->gridRow, this->gridCol, false);
        }
    }
}

void Block::onDeath() {
    this->deactivate();
}

void Block::update(float scroll, Level* lvl) {
    (void)scroll;
    (void)lvl;

    if (!this->status) return;

    if (this->destroying) {
        this->updateAnimation();
        if (this->animation.isFinished()) {
            this->deactivate();
        }
    }
}

void Block::draw(RenderWindow& window, float scroll) {
    if (!this->status) return;

    this->animation.applyToSprite(this->sprite);
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    window.draw(this->sprite);
}

void Block::updateBoundingBox() {
    int cellSize = 48;
    if (this->level != nullptr) cellSize = this->level->getCellSize();
    this->boundingBox = IntRect(0, 0, cellSize, cellSize);
}