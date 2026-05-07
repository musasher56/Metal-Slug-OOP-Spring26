#include "TextureManager.h"
#include <cstdio>

TextureManager::TextureManager() : textureCount(0) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        loaded[i] = false;
        names[i][0] = '\0';
    }
    Image img;
    img.create(1, 1, Color::White);
    fallback.loadFromImage(img);
}

TextureManager::~TextureManager() {}

bool TextureManager::loadTexture(const char* filename) {
    int idx = findSlot(filename);
    if (idx != -1) return true;
    if (textureCount >= MAX_TEXTURES) return false;

    if (!textures[textureCount].loadFromFile(filename)) {
        printf("[WARN] Texture not found: %s\n", filename);
        return false;
    }

    int i = 0;
    while (filename[i] != '\0' && i < MAX_NAME_LEN - 1) {
        names[textureCount][i] = filename[i];
        i++;
    }
    names[textureCount][i] = '\0';
    loaded[textureCount] = true;
    textureCount++;
    return true;
}



bool TextureManager::loadTexture(const char* key, const char* filepath) {
    int idx = findSlot(key);
    if (idx != -1) return true;
    if (textureCount >= MAX_TEXTURES) return false;

    if (!textures[textureCount].loadFromFile(filepath)) {
        printf("[WARN] Texture not found: %s (key: %s)\n", filepath, key);
        return false;
    }

    int i = 0;
    while (key[i] != '\0' && i < MAX_NAME_LEN - 1) {
        names[textureCount][i] = key[i];
        i++;
    }
    names[textureCount][i] = '\0';
    loaded[textureCount] = true;
    textureCount++;
    return true;
}

Texture& TextureManager::getTexture(const char* filename) {
    int idx = findSlot(filename);
    if (idx != -1) return textures[idx];

    if (loadTexture(filename)) {
        return textures[textureCount - 1];
    }
    return fallback;
}

int TextureManager::findSlot(const char* filename) const {
    for (int i = 0; i < textureCount; i++) {
        if (!loaded[i]) continue;

        int j = 0;
        bool match = true;
        while (names[i][j] != '\0' && filename[j] != '\0') {
            if (names[i][j] != filename[j]) {
                match = false;
                break;
            }
            j++;
        }
        if (match && names[i][j] == '\0' && filename[j] == '\0') {
            return i;
        }
    }
    return -1;
}
