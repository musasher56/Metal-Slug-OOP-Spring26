#include "HUD.h"
#include "CharacterManager.h"
#include "PlayerSoldier.h"
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
// Layout constants
//
// Hearts are placed at the TOP-RIGHT corner so they are always visible
// regardless of level scroll direction or platform geometry near the left edge.
//
// Panel structure (anchored to the right side):
//   HEART_W       = width  reserved for the heart sprite image
//   HEART_H       = height reserved for the heart sprite image
//   RIGHT_MARGIN  = gap from the right screen edge to the panel's right boundary
//   TOP_MARGIN    = gap from the top screen edge
//
// Score + weapon are rendered to the LEFT of the heart image inside the same panel.
// ─────────────────────────────────────────────────────────────────────────────
static const float HEART_W = 130.f;
static const float HEART_H = 50.f;
static const float RIGHT_MARGIN = 8.f;
static const float TOP_MARGIN = 8.f;
static const int   MAX_HP = 3;

// Crop rect inside each 1024x576 heart image — just the three hearts region.
static const sf::IntRect HEART_CROP(90, 40, 850, 380);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
HUD::HUD()
    : score(0), hp(MAX_HP), maxHp(MAX_HP)
    , redHueAlpha(0.f), weaponName("Pistol")
    , characterName("MARCO"), heartsLoaded(false)
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

    // Attempt to load the four heart-state images.
    // heart1.png = full health (3/3)
    // heart2.png = 2/3 health
    // heart3.png = 1/3 health
    // heart4.png = empty (0/3)
    static const char* PATHS[4] = {
        "resources/Sprites/heart1.png",
        "resources/Sprites/heart2.png",
        "resources/Sprites/heart3.png",
        "resources/Sprites/heart4.png"
    };

    this->heartsLoaded = true;
    for (int i = 0; i < 4; i++) {
        if (!this->heartTex[i].loadFromFile(PATHS[i])) {
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

    this->score = cm->getKills() * 50;
    this->hp = cm->getHealthPoints();

    // Clamp hp so the heart image index never goes out of bounds [0,3]
    if (this->hp < 0)      this->hp = 0;
    if (this->hp > MAX_HP) this->hp = MAX_HP;

    // Pull weapon name from the active player character
    PlayerSoldier* player = cm->getCurrentCharacter();
    if (player != nullptr)
        this->weaponName = player->getCurrentWeaponName();

    // Map character slot index to a display name string
    static const char* NAMES[4] = { "MARCO", "TARMA", "ERI", "FIO" };
    int idx = cm->getCurrentCharacterIdx();
    if (idx >= 0 && idx < 4)
        this->characterName = NAMES[idx];
}

// ─────────────────────────────────────────────────────────────────────────────
// draw — render the HUD panel at the TOP-RIGHT of the screen
//
// WHY right side?
//   Many Metal Slug levels start the player near x=0 on the left side of the
//   screen, and the ground terrain also has ledges and platforms there.
//   A left-anchored HUD was getting hidden behind geometry and was unreadable.
//   Right-anchoring solves this without any gameplay layout changes.
//
// WHY compute panelX each frame?
//   window.getSize().x gives the actual render-target width, which may differ
//   from the compile-time SCREEN_W constant if the window is resized.
//   One float subtraction per frame is negligible cost for correct alignment.
// ─────────────────────────────────────────────────────────────────────────────
void HUD::draw(RenderWindow& window) {

    // Derive the heart panel's left edge from the live window width
    float screenW = static_cast<float>(window.getSize().x);
    float panelX = screenW - HEART_W - RIGHT_MARGIN;  // right-side anchor
    float panelY = TOP_MARGIN;

    // The text column (score / weapon / char name) sits to the LEFT of the hearts
    float textColW = 145.f;
    float textX = panelX - textColW;        // left edge of text column
    float panelLeft = textX;                   // left edge of entire HUD panel

    // ── Dark semi-transparent backing strip ───────────────────────────────
    float totalW = textColW + HEART_W + 8.f;
    sf::RectangleShape panel(sf::Vector2f(totalW, HEART_H + 36.f));
    panel.setPosition(panelLeft - 4.f, panelY - 4.f);
    panel.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(panel);

    // ── Heart image ───────────────────────────────────────────────────────
    // Select which heart image to show based on current HP:
    //   hp=3 → imgIdx=0  (heart1.png — full)
    //   hp=2 → imgIdx=1  (heart2.png)
    //   hp=1 → imgIdx=2  (heart3.png)
    //   hp=0 → imgIdx=3  (heart4.png — empty)
    int imgIdx = MAX_HP - this->hp;
    if (imgIdx < 0) imgIdx = 0;
    if (imgIdx > 3) imgIdx = 3;

    if (this->heartsLoaded) {
        sf::Sprite heartSpr;
        heartSpr.setTexture(this->heartTex[imgIdx]);
        heartSpr.setTextureRect(HEART_CROP);
        heartSpr.setScale(
            HEART_W / static_cast<float>(HEART_CROP.width),
            HEART_H / static_cast<float>(HEART_CROP.height)
        );
        heartSpr.setPosition(panelX, panelY);
        window.draw(heartSpr);
    }
    else {
        // Fallback: simple coloured rectangles when PNG assets are missing
        for (int i = 0; i < MAX_HP; i++) {
            sf::RectangleShape pip(sf::Vector2f(30.f, HEART_H));
            pip.setPosition(panelX + static_cast<float>(i) * 38.f, panelY);
            pip.setFillColor(i < this->hp ? sf::Color(220, 30, 30) : sf::Color(60, 20, 20));
            pip.setOutlineColor(sf::Color(120, 40, 40));
            pip.setOutlineThickness(1.f);
            window.draw(pip);
        }
    }

    // ── Text column — score, weapon, character name ───────────────────────

    // Score (formatted with leading zeros)
    char scoreBuf[32];
    std::sprintf(scoreBuf, "SCORE: %06d", this->score);
    sf::Text scoreText(scoreBuf, this->font, 14);
    scoreText.setFillColor(sf::Color(255, 215, 60));   // gold
    scoreText.setPosition(textX, panelY + 2.f);
    window.draw(scoreText);

    // Current weapon
    if (this->weaponName != nullptr) {
        sf::Text wepText(this->weaponName, this->font, 12);
        wepText.setFillColor(sf::Color(180, 210, 255));  // light blue
        wepText.setPosition(textX, panelY + 22.f);
        window.draw(wepText);
    }

    // Character name
    sf::Text nameText(this->characterName, this->font, 11);
    nameText.setFillColor(sf::Color(200, 200, 200));
    nameText.setPosition(textX, panelY + 40.f);
    window.draw(nameText);

    // ── Boss Health Bar (Souls-style) ──────────────────────────────────
    // A dramatic, dark health bar at the bottom-center of the screen.
    // Inspired by Dark Souls / Elden Ring boss health bars:
    //   - Long horizontal bar spanning most of the screen width
    //   - Boss name displayed above the bar in a serif-like style
    //   - Health fill in dark red/orange that smoothly depletes
    //   - Delayed yellow "damage" trail that catches up to real HP
    //   - Thin ornamental border
    //   - Fades in when boss appears
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
// Misc setters / getters
// ─────────────────────────────────────────────────────────────────────────────
void HUD::setScore(int s) { this->score = s; }
int  HUD::getScore()  const { return this->score; }
void HUD::showDamageHue(float i) { this->redHueAlpha = i; }
void HUD::setWeaponName(const char* n) { this->weaponName = n; }

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