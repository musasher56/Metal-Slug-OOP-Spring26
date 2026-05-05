#pragma once
#include "Constants.h"

class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    Texture& getTexture(const char* key);
    bool loadTexture(const char* filepath);
    bool loadTexture(const char* key, const char* filepath);

private:
    Texture textures[MAX_TEXTURES];
    char names[MAX_TEXTURES][MAX_NAME_LEN];
    bool loaded[MAX_TEXTURES];
    int textureCount;
    Texture fallback;

    int findSlot(const char* filename) const;
};
