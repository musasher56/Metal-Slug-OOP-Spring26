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

    // Increased from 4096 to 8192 for procedural terrain (wider levels need more blocks)
    static const int MAX_MOUNTAIN_BLOCKS = 8192;
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
    void spawnIndestructibleBlock(float worldX, float worldY);
    void spawnPlatform(float startX, float startY, int count);

    // Build flat ground terrain (rows of MountainBlocks at bottom)
    void buildGroundTerrain(int surfaceRow, int depth);

    // Build mountain terrain from the hardcoded heightmap[] (survival mode)
    void buildMountainTerrain(float baseX, float baseY, int maxColHeight = 999);

    // Build terrain from a PROCEDURALLY GENERATED heightmap (campaign mode)
    // heightmap[]: one int per column = number of dirt blocks to stack upward
    // heightmapLen: number of columns in the heightmap
    // baseX, baseY: world position of the ground-level bottom-left corner
    // maxColHeight: optional cap on column height (for levels with limited vertical space)
    void buildProceduralTerrain(float baseX, float baseY, int* heightmap, int heightmapLen, int maxColHeight = 999);

    void update(float scrollX, float scrollY);
    void draw(RenderWindow& window, float scrollX, float scrollY);
    void cleanup();

    DamagableEntity** getActiveBlocks();
    int getActiveCount() const { return this->activeCount; }
    int getTotalCount()  const { return this->blockCount; }
    int getMountainCount() const { return this->mountainBlockCount; }
};

#endif
