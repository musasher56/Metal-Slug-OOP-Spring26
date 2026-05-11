#include "TextureManager.h"
#include <cstdio>





TextureManager::TextureManager() : textureCount(0) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        loaded[i] = false;
        names[i][0] = '\0';
    }
    
    
    sf::Image img;
    img.create(1, 1, Color::White);
    fallback.loadFromImage(img);
}

TextureManager::~TextureManager() {}





int TextureManager::findSlot(const char* key) const {
    for (int i = 0; i < textureCount; i++) {
        if (!loaded[i]) continue;
        int j = 0;
        bool match = true;
        while (names[i][j] != '\0' && key[j] != '\0') {
            if (names[i][j] != key[j]) { match = false; break; }
            j++;
        }
        if (match && names[i][j] == '\0' && key[j] == '\0') return i;
    }
    return -1;
}



static void registerKey(char names[][MAX_NAME_LEN], bool* loaded,
    int slot, const char* key)
{
    int i = 0;
    while (key[i] != '\0' && i < MAX_NAME_LEN - 1) {
        names[slot][i] = key[i];
        i++;
    }
    names[slot][i] = '\0';
    loaded[slot] = true;
}





bool TextureManager::loadTexture(const char* filename) {
    if (findSlot(filename) != -1)  return true;
    if (textureCount >= MAX_TEXTURES) return false;

    if (!textures[textureCount].loadFromFile(filename)) {
        printf("[WARN] Texture not found: %s\n", filename);
        return false;
    }
    registerKey(names, loaded, textureCount, filename);
    textureCount++;
    return true;
}

bool TextureManager::loadTexture(const char* key, const char* filepath) {
    if (findSlot(key) != -1) return true;
    if (textureCount >= MAX_TEXTURES) return false;

    if (!textures[textureCount].loadFromFile(filepath)) {
        printf("[WARN] Texture not found: %s (key: %s)\n", filepath, key);
        return false;
    }
    registerKey(names, loaded, textureCount, key);
    textureCount++;
    return true;
}

Texture& TextureManager::getTexture(const char* key) {
    int idx = findSlot(key);
    if (idx != -1) return textures[idx];

    
    if (loadTexture(key)) return textures[textureCount - 1];
    return fallback;
}


















Texture& TextureManager::loadTextureWithMask(const char* key,
    const char* filepath,
    sf::Color   maskColor,
    sf::Uint8   tolerance)
{
    
    int idx = findSlot(key);
    if (idx != -1) return textures[idx];

    if (textureCount >= MAX_TEXTURES) {
        printf("[WARN] loadTextureWithMask: texture pool full, key=%s\n", key);
        return fallback;
    }

    
    sf::Image img;
    if (!img.loadFromFile(filepath)) {
        printf("[WARN] loadTextureWithMask: file missing: %s\n", filepath);
        return fallback;
    }

    
    
    
    sf::Vector2u size = img.getSize();
    const int tol = static_cast<int>(tolerance);

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            sf::Color px = img.getPixel(x, y);

            int dr = static_cast<int>(px.r) - static_cast<int>(maskColor.r);
            int dg = static_cast<int>(px.g) - static_cast<int>(maskColor.g);
            int db = static_cast<int>(px.b) - static_cast<int>(maskColor.b);
            if (dr < 0) dr = -dr;
            if (dg < 0) dg = -dg;
            if (db < 0) db = -db;

            
            int dist = dr > dg ? dr : dg;
            if (db > dist) dist = db;

            if (dist <= tol) {
                img.setPixel(x, y, sf::Color(px.r, px.g, px.b, 0));
            }
        }
    }

    
    int slot = textureCount;   
    if (!textures[slot].loadFromImage(img)) {
        printf("[WARN] loadTextureWithMask: GPU upload failed for %s\n", filepath);
        return fallback;
    }

    
    registerKey(names, loaded, slot, key);
    textureCount++;   

    printf("[INFO] Loaded '%s' with colour-key mask (%ux%u px, tol=%d)\n",
        key, size.x, size.y, tol);

    return textures[slot];   
}










bool TextureManager::makeColorTexture(const char* key, sf::Color color) {
    if (findSlot(key) != -1) return true;           
    if (textureCount >= MAX_TEXTURES) return false;

    sf::Image img;
    img.create(4, 4, color);    
    if (!textures[textureCount].loadFromImage(img)) return false;

    registerKey(names, loaded, textureCount, key);
    textureCount++;
    return true;
}