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
static const float HEART_W       = 130.f;
static const float HEART_H       = 50.f;
static const float RIGHT_MARGIN  = 8.f;
static const float TOP_MARGIN    = 8.f;
static const int   MAX_HP        = 3;

// Crop rect inside each 1024x576 heart image — just the three hearts region.
static const sf::IntRect HEART_CROP(90, 40, 850, 380);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
HUD::HUD()
    : score(0), hp(MAX_HP), maxHp(MAX_HP)
    , redHueAlpha(0.f), weaponName("Pistol")
    , characterName("MARCO"), heartsLoaded(false)
{
    if (!this->font.loadFromFile("resources/Fonts/arial.ttf")) { }

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
    this->hp    = cm->getHealthPoints();

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
    float panelX  = screenW - HEART_W - RIGHT_MARGIN;  // right-side anchor
    float panelY  = TOP_MARGIN;

    // The text column (score / weapon / char name) sits to the LEFT of the hearts
    float textColW = 145.f;
    float textX    = panelX - textColW;        // left edge of text column
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
            HEART_W  / static_cast<float>(HEART_CROP.width),
            HEART_H  / static_cast<float>(HEART_CROP.height)
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
}

// ─────────────────────────────────────────────────────────────────────────────
// Misc setters / getters
// ─────────────────────────────────────────────────────────────────────────────
void HUD::setScore(int s)              { this->score = s; }
int  HUD::getScore()  const            { return this->score; }
void HUD::showDamageHue(float i)       { this->redHueAlpha = i; }
void HUD::setWeaponName(const char* n) { this->weaponName = n; }