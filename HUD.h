#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class CharacterManager;


// ─────────────────────────────────────────────────────────────────────────────
// HUD — Heads-Up Display
//
// Renders the in-game overlay: heart display (single sprite from 4 PNGs),
// boss health bar (Souls-style), and "GREAT ENEMY FELLED" message.
//
// Heart display uses 4 PNGs:
//   heart1.png = 3 filled hearts  (HP = 3)
//   heart2.png = 2 filled hearts  (HP = 2)
//   heart3.png = 1 filled heart   (HP = 1)
//   heart4.png = 0 filled hearts  (HP = 0, all empty)
//
// The correct PNG is selected each frame based on current player HP.
// ─────────────────────────────────────────────────────────────────────────────
class HUD {
private:
    int          hp;
    int          maxHp;
    Font         font;
    float        redHueAlpha;

    // Heart textures: one per HP state
    // heartTex[0] = heart1.png (3 hearts filled)
    // heartTex[1] = heart2.png (2 hearts filled)
    // heartTex[2] = heart3.png (1 heart filled)
    // heartTex[3] = heart4.png (0 hearts filled, all empty)
    Texture      heartTex[4];
    bool         heartsLoaded;

    // ── Boss health bar (Souls-style) ──
    float        bossHealthFraction;      // current boss HP (0.0 – 1.0)
    float        bossHealthDisplayed;     // smoothly animated displayed HP
    const char* bossName;               // boss name for display
    bool         bossBarVisible;          // true when a boss is active
    float        bossBarAppearTimer;      // animation timer for bar appearing
    float        bossBarAlpha;            // fade-in alpha

    // ── "GREAT ENEMY FELLED" overlay (Elden Ring style) ──
    bool         felledVisible;           // true while the felled message is on screen
    int          felledPhase;             // 0=fade in, 1=hold, 2=fade out
    float        felledTimer;             // seconds elapsed in current phase
    float        felledAlpha;             // current alpha (0–255)
    const char* felledBossName;          // name of the defeated boss

public:
    HUD();
    virtual ~HUD();

    void update(CharacterManager* cm, int levelNum);
    void draw(RenderWindow& window);
    void showDamageHue(float intensity);

    // Boss health bar control
    void setBossInfo(const char* name, float healthFrac);
    void clearBossInfo();

    // Boss felled message
    void showBossFelled(const char* bossName);
    bool isFelledShowing() const;

private:
    void drawBossFelled(RenderWindow& window);
};
