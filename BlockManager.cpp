#include "BlockManager.h"
#include "Block.h"
#include "Level.h"

BlockManager::BlockManager(TextureManager* texMgr, AudioManager* audMgr, Level* lvl)
    : blockCount(0)
    , texMgr(texMgr)
    , audMgr(audMgr)
    , level(lvl)
    , activeCount(0)
{
    for (int i = 0; i < MAX_BLOCKS; i++) {
        this->blocks[i] = nullptr;
        this->activeCache[i] = nullptr;
    }
}

BlockManager::~BlockManager() {
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr) {
            delete this->blocks[i];
            this->blocks[i] = nullptr;
        }
    }
    this->blockCount = 0;
    this->activeCount = 0;
}

void BlockManager::spawnBlock(float worldX, float worldY) {
    if (this->blockCount >= MAX_BLOCKS) return;
    if (this->level == nullptr) return;

    int cellSize = this->level->getCellSize();

    int col = static_cast<int>(worldX / cellSize);
    int row = static_cast<int>(worldY / cellSize);

    // Bounds check
    if (col < 0 || col >= this->level->getWidth()) return;
    if (row < 0 || row >= this->level->getHeight()) return;

    // Skip if cell already has a block (check existing blocks, not the grid —
    // blocks no longer mark the level grid solid; see Block.cpp for why).
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] == nullptr || !this->blocks[i]->getStatus()) continue;
        // Compare grid position by snapping worldX/worldY to col/row
        // The Block constructor already does this math; reuse it here.
        int existCol = static_cast<int>(this->blocks[i]->getPosition().x / cellSize);
        int existRow = static_cast<int>(this->blocks[i]->getPosition().y / cellSize);
        if (existCol == col && existRow == row) return;  // already occupied
    }

    Block* b = new Block(this->texMgr, this->audMgr, worldX, worldY, this->level);
    if (b != nullptr) {
        this->blocks[this->blockCount++] = b;
    }
}

void BlockManager::spawnPlatform(float startX, float startY, int count) {
    if (this->level == nullptr) return;
    int cellSize = this->level->getCellSize();

    for (int i = 0; i < count; i++) {
        float bx = startX + static_cast<float>(i * cellSize);
        this->spawnBlock(bx, startY);
    }
}

void BlockManager::update(float scroll) {
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr && this->blocks[i]->getStatus()) {
            this->blocks[i]->update(scroll, this->level);
        }
    }
}

void BlockManager::draw(RenderWindow& window, float scroll) {
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr) {
            this->blocks[i]->draw(window, scroll);
        }
    }
}

void BlockManager::cleanup() {
    int i = 0;
    while (i < this->blockCount) {
        Block* b = this->blocks[i];
        if (b == nullptr || !b->getStatus()) {
            if (b != nullptr) {
                delete b;
                this->blocks[i] = nullptr;
            }
            this->blockCount--;
            if (i < this->blockCount) {
                this->blocks[i] = this->blocks[this->blockCount];
                this->blocks[this->blockCount] = nullptr;
            }
        }
        else {
            i++;
        }
    }
}

DamagableEntity** BlockManager::getActiveBlocks() {
    this->activeCount = 0;
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr &&
            this->blocks[i]->getStatus() &&
            this->blocks[i]->isAlive())
        {
            this->activeCache[this->activeCount] = this->blocks[i];
            this->activeCount++;
        }
    }
    return this->activeCache;
}