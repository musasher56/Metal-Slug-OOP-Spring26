#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "Constants.h"

class Block;
class Level;
class TextureManager;
class AudioManager;
class DamagableEntity;

class BlockManager {
public:
    static const int MAX_BLOCKS = 128;

private:
    Block* blocks[MAX_BLOCKS];
    int blockCount;
    TextureManager* texMgr;
    AudioManager* audMgr;
    Level* level;

    DamagableEntity* activeCache[MAX_BLOCKS];
    int activeCount;

public:
    BlockManager(TextureManager* texMgr, AudioManager* audMgr, Level* lvl);
    ~BlockManager();

    void spawnBlock(float worldX, float worldY);
    void spawnPlatform(float startX, float startY, int count);

    void update(float scroll);
    void draw(RenderWindow& window, float scroll);
    void cleanup();

    DamagableEntity** getActiveBlocks();
    int getActiveCount() const { return this->activeCount; }
};

#endif