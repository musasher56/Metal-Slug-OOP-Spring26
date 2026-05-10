#include "HUD.h"
#include "CharacterManager.h"
#include "PlayerSoldier.h"
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
// Layout constants
//
// Heart display is placed at the TOP-LEFT corner using a single sprite
// selected from 4 PNGs based on current HP:
//   heart1.png = 3 hearts filled (HP 3)
//   heart2.png = 2 hearts filled (HP 2)
//   heart3.png = 1 heart filled  (HP 1)
//   heart4.png = 0 hearts filled (HP 0)
// ─────────────────────────────────────────────────────────────────────────────
static const float HEART_DRAW_SCALE = 0.2f;   // scale for the heart sprite
static const float HEART_MARGIN_X = 10.f;
static const float HEART_MARGIN_Y = 8.f;
static const int   MAX_HP = 3;

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
HUD::HUD()
    : hp(MAX_HP), maxHp(MAX_HP)
    , redHueAlpha(0.f), heartsLoaded(false)
    , bossHealthFraction(0.f), bossHealthDisplayed(0.f), bossName(nullptr)
    , bossBarVisible(false), bossBarAppearTimer(0.f), bossBarAlpha(0.f)
    , felledVisible(false), felledPhase(0), felledTimer(0.f)
    , felledAlpha(0.f), felledBossName(nullptr)
{
    // Try multiple font paths — SFML renders NOTHING if the font fails to load
    bool fontLoaded = this->font.loadFromFile("resources/Fonts/arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("C:\\Windows\\Fonts\\arialunicodems.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("resources/font.ttf");

    // Load the 4 heart state textures:
    // heart1.png = all 3 hearts (full HP)
    // heart2.png = 2 hearts
    // heart3.png = 1 heart
    // heart4.png = 0 hearts (all empty)
    static const char* HEART_PATHS[4] = {
        "resources/Sprites/heart1.png",
        "resources/Sprites/heart2.png",
        "resources/Sprites/heart3.png",
        "resources/Sprites/heart4.png"
    };

    this->heartsLoaded = true;
    for (int i = 0; i < 4; i++) {
        sf::Image img;
        if (!img.loadFromFile(HEART_PATHS[i])) {
            this->heartsLoaded = false;
            continue;
        }
        // Simple mask: make pure black pixels transparent
        img.createMaskFromColor(sf::Color::Black);
        if (!this->heartTex[i].loadFromImage(img)) {
            this->heartsLoaded = false;
        }
    }
}

HUD::~HUD() {}

// ─────────────────────────────────────────────────────────────────────────────
// update — pull current game state into the HUD each tick
// ─────────────────────────────────────────────────────────────────────────────
void HUD::update(CharacterManager* cm, int levelNum) {
    (void)levelNum;
    if (cm == nullptr) return;

    this->hp = cm->getHealthPoints();

    // Pull maxHp from the active player character
    PlayerSoldier* player = cm->getCurrentCharacter();
    if (player != nullptr) {
        this->maxHp = player->getMaxHealth();
        if (this->maxHp < 1) this->maxHp = 1;
    }

    // Clamp hp so the heart index never goes out of bounds
    if (this->hp < 0) this->hp = 0;
    if (this->hp > this->maxHp) this->hp = this->maxHp;
}

// ─────────────────────────────────────────────────────────────────────────────
// draw — render the HUD
//
// Heart display: a single sprite at top-left, selected from 4 PNGs
// based on current HP. Simple, clean, no individual pip drawing.
// ─────────────────────────────────────────────────────────────────────────────
void HUD::draw(RenderWindow& window) {

    // ── Heart display ────────────────────────────────────────────────────
    // Select the correct heart PNG based on current HP:
    //   HP 3 → heart1.png (index 0)
    //   HP 2 → heart2.png (index 1)
    //   HP 1 → heart3.png (index 2)
    //   HP 0 → heart4.png (index 3)
    if (this->heartsLoaded) {
        int texIdx = this->maxHp - this->hp;  // 0=full, maxHp=empty
        if (texIdx < 0) texIdx = 0;
        if (texIdx > 3) texIdx = 3;

        sf::Sprite heartSprite;
        heartSprite.setTexture(this->heartTex[texIdx]);
        sf::Vector2u texSize = this->heartTex[texIdx].getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            heartSprite.setTextureRect(sf::IntRect(0, 0, (int)texSize.x, (int)texSize.y));
        }
        heartSprite.setScale(HEART_DRAW_SCALE, HEART_DRAW_SCALE);
        heartSprite.setPosition(HEART_MARGIN_X, HEART_MARGIN_Y);
        window.draw(heartSprite);
    }
    else {
        // Fallback: simple colored rectangle when PNG assets are missing
        sf::RectangleShape heartBox(sf::Vector2f(120.f, 30.f));
        heartBox.setPosition(HEART_MARGIN_X, HEART_MARGIN_Y);
        heartBox.setFillColor(sf::Color(40, 10, 10));
        heartBox.setOutlineColor(sf::Color(120, 40, 40));
        heartBox.setOutlineThickness(1.f);
        window.draw(heartBox);

        // Draw small filled rectangles for each HP
        for (int i = 0; i < this->hp && i < this->maxHp; i++) {
            sf::RectangleShape pip(sf::Vector2f(30.f, 22.f));
            pip.setPosition(HEART_MARGIN_X + 5.f + (float)i * 38.f, HEART_MARGIN_Y + 4.f);
            pip.setFillColor(sf::Color(220, 30, 30));
            window.draw(pip);
        }
    }

    // ── Boss Health Bar (Souls-style) ──────────────────────────────────
    if (this->bossBarVisible && this->bossName != nullptr) {
        // Animate appearance
        this->bossBarAppearTimer += 1.f / 60.f;
        if (this->bossBarAlpha < 255.f) {
            this->bossBarAlpha += 5.f;  // fade in over ~1 second
            if (this->bossBarAlpha > 255.f) this->bossBarAlpha = 255.f;
        }

        // Smoothly animate the displayed health toward the actual health
        // This creates the Souls-like "delayed drain" effect
        if (this->bossHealthDisplayed > this->bossHealthFraction) {
            this->bossHealthDisplayed -= 0.004f;  // slow drain speed
            if (this->bossHealthDisplayed < this->bossHealthFraction)
                this->bossHealthDisplayed = this->bossHealthFraction;
        }
        else {
            this->bossHealthDisplayed = this->bossHealthFraction;
        }

        float barW = 700.f;
        float barH = 14.f;
        float barX = ((float)SCREEN_W - barW) / 2.f;
        float barY = (float)SCREEN_H - 70.f;
        float nameY = barY - 30.f;

        sf::Uint8 alpha = static_cast<sf::Uint8>(this->bossBarAlpha);

        // ── Dark backing strip behind the entire bar area ──
        sf::RectangleShape backing(sf::Vector2f(barW + 20.f, barH + 50.f));
        backing.setPosition(barX - 10.f, nameY - 8.f);
        backing.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha * 0.5f)));
        window.draw(backing);

        // ── Boss name ──
        sf::Text bossNameText(this->bossName, this->font, 22);
        bossNameText.setFillColor(sf::Color(220, 190, 130, alpha));
        bossNameText.setStyle(sf::Text::Bold);
        sf::FloatRect bn = bossNameText.getLocalBounds();
        bossNameText.setOrigin(bn.width / 2.f, 0.f);
        bossNameText.setPosition((float)SCREEN_W / 2.f, nameY);
        window.draw(bossNameText);

        // ── Outer border (ornamental thin line) ──
        sf::RectangleShape border(sf::Vector2f(barW + 4.f, barH + 4.f));
        border.setPosition(barX - 2.f, barY - 2.f);
        border.setFillColor(sf::Color(0, 0, 0, 0));
        border.setOutlineColor(sf::Color(120, 100, 60, alpha));
        border.setOutlineThickness(1.f);
        window.draw(border);

        // ── Background bar (dark gray) ──
        sf::RectangleShape bgBar(sf::Vector2f(barW, barH));
        bgBar.setPosition(barX, barY);
        bgBar.setFillColor(sf::Color(30, 20, 15, alpha));
        window.draw(bgBar);

        // ── Damage trail (yellow/orange, delayed) ──
        if (this->bossHealthDisplayed > this->bossHealthFraction) {
            float trailW = barW * this->bossHealthDisplayed;
            sf::RectangleShape trailBar(sf::Vector2f(trailW, barH));
            trailBar.setPosition(barX, barY);
            trailBar.setFillColor(sf::Color(200, 160, 40, alpha));
            window.draw(trailBar);
        }

        // ── Actual health bar (dark red / crimson) ──
        float fillW = barW * this->bossHealthFraction;
        if (fillW > 0.f) {
            sf::RectangleShape healthBar(sf::Vector2f(fillW, barH));
            healthBar.setPosition(barX, barY);
            // Crimson red, gets darker as health decreases
            int r = 160 + static_cast<int>(60.f * this->bossHealthFraction);
            int g = static_cast<int>(40.f * this->bossHealthFraction);
            int b = static_cast<int>(20.f * this->bossHealthFraction);
            healthBar.setFillColor(sf::Color(r, g, b, alpha));
            window.draw(healthBar);
        }

        // ── Thin separator line at the top of the bar ──
        sf::RectangleShape topLine(sf::Vector2f(barW, 1.f));
        topLine.setPosition(barX, barY);
        topLine.setFillColor(sf::Color(180, 150, 80, alpha));
        window.draw(topLine);
    }

    // ── "GREAT ENEMY FELLED" overlay (Elden Ring style) ──
    this->drawBossFelled(window);
}

// ─────────────────────────────────────────────────────────────────────────────
// Misc setters
// ─────────────────────────────────────────────────────────────────────────────
void HUD::showDamageHue(float i) { this->redHueAlpha = i; }

void HUD::setBossInfo(const char* name, float healthFrac) {
    this->bossName = name;
    this->bossHealthFraction = healthFrac;
    if (!this->bossBarVisible) {
        this->bossBarVisible = true;
        this->bossBarAppearTimer = 0.f;
        this->bossBarAlpha = 0.f;
        this->bossHealthDisplayed = healthFrac;  // start fully filled
    }
}

void HUD::clearBossInfo() {
    this->bossBarVisible = false;
    this->bossName = nullptr;
    this->bossHealthFraction = 0.f;
    this->bossHealthDisplayed = 0.f;
    this->bossBarAlpha = 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// showBossFelled — triggers the Elden Ring-style "GREAT ENEMY FELLED" overlay
//
// Phases:
//   0 = fade in  (1.5s — slow dramatic reveal)
//   1 = hold     (3.0s — message stays visible)
//   2 = fade out (2.0s — slow dissolve)
// ─────────────────────────────────────────────────────────────────────────────
void HUD::showBossFelled(const char* bossName) {
    this->felledVisible = true;
    this->felledPhase = 0;
    this->felledTimer = 0.f;
    this->felledAlpha = 0.f;
    this->felledBossName = bossName;
}

bool HUD::isFelledShowing() const {
    return this->felledVisible;
}

void HUD::drawBossFelled(RenderWindow& window) {
    if (!this->felledVisible) return;

    // Timing constants (seconds)
    const float FADE_IN_DUR = 1.5f;
    const float HOLD_DUR = 3.0f;
    const float FADE_OUT_DUR = 2.0f;

    this->felledTimer += 1.f / 60.f;

    // ── Phase state machine ──
    if (this->felledPhase == 0) {
        // Fade in
        this->felledAlpha = (this->felledTimer / FADE_IN_DUR) * 255.f;
        if (this->felledAlpha >= 255.f) {
            this->felledAlpha = 255.f;
            this->felledPhase = 1;
            this->felledTimer = 0.f;
        }
    }
    else if (this->felledPhase == 1) {
        // Hold
        this->felledAlpha = 255.f;
        if (this->felledTimer >= HOLD_DUR) {
            this->felledPhase = 2;
            this->felledTimer = 0.f;
        }
    }
    else if (this->felledPhase == 2) {
        // Fade out
        this->felledAlpha = 255.f * (1.f - this->felledTimer / FADE_OUT_DUR);
        if (this->felledAlpha <= 0.f) {
            this->felledAlpha = 0.f;
            this->felledVisible = false;
            return;
        }
    }

    sf::Uint8 alpha = static_cast<sf::Uint8>(this->felledAlpha);

    float centerX = (float)SCREEN_W / 2.f;
    float centerY = (float)SCREEN_H * 0.38f;

    // ── Full-screen dark overlay (subtle vignette) ──
    sf::RectangleShape vignette(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
    vignette.setPosition(0.f, 0.f);
    vignette.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha * 0.35f)));
    window.draw(vignette);

    // ── Horizontal ornamental lines above and below text ──
    float lineW = 500.f;
    float lineH = 2.f;
    sf::RectangleShape topLine(sf::Vector2f(lineW, lineH));
    topLine.setPosition(centerX - lineW / 2.f, centerY - 10.f);
    topLine.setFillColor(sf::Color(180, 150, 60, alpha));
    window.draw(topLine);

    // ── "GREAT ENEMY FELLED" main text ──
    sf::Text felledText("GREAT ENEMY FELLED", this->font, 36);
    felledText.setFillColor(sf::Color(255, 220, 100, alpha));
    felledText.setStyle(sf::Text::Bold);
    sf::FloatRect fb = felledText.getLocalBounds();
    felledText.setOrigin(fb.left + fb.width / 2.f, fb.top + fb.height / 2.f);
    felledText.setPosition(centerX, centerY + 12.f);
    window.draw(felledText);

    // ── Boss name below the main text ──
    if (this->felledBossName != nullptr) {
        sf::Text nameText(this->felledBossName, this->font, 22);
        nameText.setFillColor(sf::Color(200, 170, 80, static_cast<sf::Uint8>(alpha * 0.8f)));
        sf::FloatRect nb = nameText.getLocalBounds();
        nameText.setOrigin(nb.left + nb.width / 2.f, nb.top + nb.height / 2.f);
        nameText.setPosition(centerX, centerY + 50.f);
        window.draw(nameText);
    }

    // ── Bottom ornamental line ──
    sf::RectangleShape botLine(sf::Vector2f(lineW, lineH));
    botLine.setPosition(centerX - lineW / 2.f, centerY + 72.f);
    botLine.setFillColor(sf::Color(180, 150, 60, alpha));
    window.draw(botLine);
}
