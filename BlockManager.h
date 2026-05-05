#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "Constants.h"
#include "Block.h"

class Level;
class TextureManager;
class AudioManager;
class DamagableEntity;

class BlockManager {
public:
    static const int MAX_BLOCKS = 128;

    // =====================================================================
    // MOUNTAIN TERRAIN
    // =====================================================================
    static const int MAX_MOUNTAIN_BLOCKS = 4096;
    static const int MOUNTAIN_COL_STEP = 48;
    static const int MOUNTAIN_HEIGHTMAP_LEN = 190;
    static const int heightmap[MOUNTAIN_HEIGHTMAP_LEN];

private:
    Block* blocks[MAX_BLOCKS];
    int blockCount;
    TextureManager* texMgr;
    AudioManager* audMgr;
    Level* level;

    DamagableEntity* activeCache[MAX_BLOCKS];
    int activeCount;

    MountainBlock* mountainBlocks[MAX_MOUNTAIN_BLOCKS];
    int mountainBlockCount;

public:
    BlockManager(TextureManager* texMgr, AudioManager* audMgr, Level* lvl);
    ~BlockManager();

    void spawnBlock(float worldX, float worldY);
    void spawnPlatform(float startX, float startY, int count);

    // Fill rows with visible dirt blocks for ground floor
    void buildGroundTerrain(int surfaceRow, int depth);

    void buildMountainTerrain(float baseX, float baseY);

    void update(float scrollX, float scrollY);
    void draw(RenderWindow& window, float scrollX, float scrollY);
    void cleanup();

    DamagableEntity** getActiveBlocks();
    int getActiveCount() const { return this->activeCount; }
    int getTotalCount()  const { return this->blockCount; }
};

#endif