#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class CharacterManager;


// ─────────────────────────────────────────────────────────────────────────────
// HUD — Heads-Up Display
//
// Renders the in-game overlay: character portrait + name, health bar,
// score, and current weapon.  All drawing is done in screen space (no
// scroll offset) using a fixed-size panel anchored at the top-left corner.
//
// WHY a separate draw pass after the game world?
//   The SFML render order means HUD shapes always appear ON TOP of world
//   geometry without needing a z-buffer.  We simply call hud->draw() last
//   inside PlayState::render() after all world objects have been drawn.
//
// Memory: HUD owns no heap allocations — all shapes are value members or
// stack-allocated in draw().  No Rule-of-Three needed.
// ─────────────────────────────────────────────────────────────────────────────
class HUD {
private:
    int          score;
    int          hp;
    int          maxHp;
    Clock        elapsedTime;
    Font         font;
    float        redHueAlpha;
    const char* weaponName;
    const char* characterName;

    // Four heart sprites representing health states:
    // heartTex[0] = heart1.png (3/3 — full)
    // heartTex[1] = heart2.png (2/3)
    // heartTex[2] = heart3.png (1/3)
    // heartTex[3] = heart4.png (0/3 — empty)
    // Selection: index = MAX_HEARTS - hp  (hp=3→0, hp=2→1, hp=1→2, hp=0→3)
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
    void setScore(int s);
    int  getScore() const;
    void showDamageHue(float intensity);
    void setWeaponName(const char* name);

    // Boss health bar control
    void setBossInfo(const char* name, float healthFrac);
    void clearBossInfo();

    // Boss felled message
    void showBossFelled(const char* bossName);
    bool isFelledShowing() const;

private:
    void drawBossFelled(RenderWindow& window);
};