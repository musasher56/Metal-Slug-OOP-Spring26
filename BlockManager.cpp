#include "BlockManager.h"
#include "Level.h"
#include "TextureManager.h"




const int BlockManager::heightmap[BlockManager::MOUNTAIN_HEIGHTMAP_LEN] = {

     1, 1, 1, 1, 1, 1, 1, 1, 1, 1,


      1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
      6, 6, 7, 7, 8, 8, 9, 9,10,10,
     11,11,12,12,13,13,14,14,15,15,
     16,16,17,17,18,18,19,19,20,20,
     21,21,22,22,23,23,24,24,25,25,


     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,
     25,25,25,25,25,25,25,25,25,25,


      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


BlockManager::BlockManager(TextureManager* texMgr, AudioManager* audMgr, Level* lvl)
    : blockCount(0)
    , texMgr(texMgr)
    , audMgr(audMgr)
    , level(lvl)
    , activeCount(0)
    , mountainBlockCount(0)
{
    for (int i = 0; i < MAX_BLOCKS; i++) {
        this->blocks[i] = nullptr;
        this->activeCache[i] = nullptr;
    }
    for (int i = 0; i < MAX_MOUNTAIN_BLOCKS; i++) {
        this->mountainBlocks[i] = nullptr;
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

    for (int i = 0; i < this->mountainBlockCount; i++) {
        if (this->mountainBlocks[i] != nullptr) {
            delete this->mountainBlocks[i];
            this->mountainBlocks[i] = nullptr;
        }
    }
    this->mountainBlockCount = 0;
}

void BlockManager::spawnBlock(float worldX, float worldY) {
    if (this->blockCount >= MAX_BLOCKS) return;
    if (this->level == nullptr) return;

    int cellSize = this->level->getCellSize();

    int col = static_cast<int>(worldX / cellSize);
    int row = static_cast<int>(worldY / cellSize);

    if (col < 0 || col >= this->level->getWidth()) return;
    if (row < 0 || row >= this->level->getHeight()) return;

    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] == nullptr || !this->blocks[i]->getStatus()) continue;
        int existCol = static_cast<int>(this->blocks[i]->getPosition().x / cellSize);
        int existRow = static_cast<int>(this->blocks[i]->getPosition().y / cellSize);
        if (existCol == col && existRow == row) return;
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





void BlockManager::buildGroundTerrain(int surfaceRow, int depth) {
    if (this->level == nullptr) return;

    int cellSize = this->level->getCellSize();

    for (int rowOff = 0; rowOff < depth; rowOff++) {
        int row = surfaceRow + rowOff;
        if (row >= this->level->getHeight()) break;

        float wy = static_cast<float>(row) * cellSize;

        for (int col = 0; col < this->level->getWidth()
            && this->mountainBlockCount < MAX_MOUNTAIN_BLOCKS; col++) {
            float wx = static_cast<float>(col) * cellSize;

            MountainBlock* mb = new MountainBlock(this->texMgr, wx, wy);
            this->mountainBlocks[this->mountainBlockCount++] = mb;

            this->level->setSolid(row, col, true);
        }
    }
}

void BlockManager::buildMountainTerrain(float baseX, float baseY) {
    int bsz = MountainBlock::BLOCK_SIZE;
    int cellSize = 48;

    for (int col = 0; col < MOUNTAIN_HEIGHTMAP_LEN && this->mountainBlockCount < MAX_MOUNTAIN_BLOCKS; col++) {
        int colHeight = heightmap[col];
        float wx = baseX + col * MOUNTAIN_COL_STEP;

        for (int row = 0; row < colHeight && this->mountainBlockCount < MAX_MOUNTAIN_BLOCKS; row++) {
            float wy = baseY - (row + 1) * bsz;

            MountainBlock* mb = new MountainBlock(this->texMgr, wx, wy);
            this->mountainBlocks[this->mountainBlockCount++] = mb;

            if (this->level != nullptr) {
                int gc = static_cast<int>(wx + bsz * 0.5f) / cellSize;
                int gr = static_cast<int>(wy + bsz * 0.5f) / cellSize;
                this->level->setSolid(gr, gc, true);
            }
        }
    }
}

void BlockManager::update(float scrollX, float scrollY) {
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr && this->blocks[i]->getStatus()) {
            this->blocks[i]->update(scrollX, this->level);
        }
    }
}

void BlockManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    for (int i = 0; i < this->blockCount; i++) {
        if (this->blocks[i] != nullptr) {
            this->blocks[i]->draw(window, scrollX, scrollY);
        }
    }
    for (int i = 0; i < this->mountainBlockCount; i++) {
        if (this->mountainBlocks[i] != nullptr && this->mountainBlocks[i]->getActive()) {
            this->mountainBlocks[i]->draw(window, scrollX, scrollY);
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