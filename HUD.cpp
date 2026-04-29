#include "HUD.h"
#include "CharacterManager.h"

HUD::HUD() : score(0), hp(3), redHueAlpha(0.0f) {
    // WHY: Load default font
    if (!this->font.loadFromFile("resources/Fonts/arial.ttf")) {
        // Fallback to default if custom font fails
    }
}

HUD::~HUD() {
}

void HUD::update(CharacterManager* cm, int levelNum) {
    // WHY: Update HUD stats from character manager
    if (cm != nullptr) {
        this->score = cm->getKills() * 50;  // Simplified score
        this->hp = cm->getHealthPoints();
    }
}

void HUD::draw(RenderWindow& window) {
    // WHY: Render HUD elements
    Text scoreText("Score: " + std::to_string(this->score), this->font, 24);
    scoreText.setPosition(10, 10);
    scoreText.setFillColor(Color::White);
    window.draw(scoreText);
    
    Text hpText("HP: " + std::to_string(this->hp), this->font, 24);
    hpText.setPosition(10, 40);
    hpText.setFillColor(Color::White);
    window.draw(hpText);
}

void HUD::setScore(int s) {
    this->score = s;
}

int HUD::getScore() const {
    return this->score;
}

void HUD::showDamageHue(float intensity) {
    this->redHueAlpha = intensity;
}
