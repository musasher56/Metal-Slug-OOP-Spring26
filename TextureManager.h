#pragma once
#include "Constants.h"

class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    Texture& getTexture(const char* key);
    bool     loadTexture(const char* filepath);
    bool     loadTexture(const char* key, const char* filepath);

    // ── Color-key masked loader ───────────────────────────────────────────────
    // Loads a PNG with a solid background (e.g. black) and makes every pixel
    // within `tolerance` (Chebyshev distance) of `maskColor` fully transparent.
    //
    // Returns a direct Texture& — no follow-up getTexture() call needed.
    // Returns the 1×1 white fallback on failure (never a dangling reference).
    //
    // WHY return Texture& instead of bool:
    //   The bool + getTexture() two-step pattern requires a second findSlot()
    //   call, which creates a subtle point of failure if the key lookup has any
    //   edge case.  Returning the texture reference directly is safer and faster.
    Texture& loadTextureWithMask(const char* key,
        const char* filepath,
        sf::Color   maskColor = sf::Color::Black,
        sf::Uint8   tolerance = 50);

    // Creates a solid-colour 1×1 texture registered under `key`.
    // Used as a placeholder when a real sprite file is missing (e.g. blast.png).
    bool makeColorTexture(const char* key, sf::Color color);

private:
    Texture textures[MAX_TEXTURES];
    char    names[MAX_TEXTURES][MAX_NAME_LEN];
    bool    loaded[MAX_TEXTURES];
    int     textureCount;
    Texture fallback;          // 1×1 white — returned on any load failure

    int findSlot(const char* key) const;
};