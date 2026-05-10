#include "HUD.h"
#include "CharacterManager.h"
#include "PlayerSoldier.h"

HUD::HUD() : score(0), hp(3), redHueAlpha(0.0f), weaponName("Pistol") {

    if (!this->font.loadFromFile("resources/Fonts/arial.ttf")) {

    }
}

HUD::~HUD() {
}

void HUD::update(CharacterManager* cm, int levelNum) {

    if (cm != nullptr) {
        this->score = cm->getKills() * 50;
        this->hp = cm->getHealthPoints();
        PlayerSoldier* player = cm->getCurrentCharacter();
        if (player != nullptr) {
            this->weaponName = player->getCurrentWeaponName();
        }
    }
}

void HUD::draw(RenderWindow& window) {

    // Draw score
    Text scoreText("Score: " + std::to_string(this->score), this->font, 24);
    scoreText.setPosition(10, 10);
    scoreText.setFillColor(Color::White);
    window.draw(scoreText);

    // Draw hearts
    for (int i = 0; i < 3; i++) {
        float x = 10.f + i * 30.f;
        float y = 40.f;

        if (i < this->hp) {
            // Full heart
            CircleShape heart(10.f);
            heart.setPosition(x, y);
            heart.setFillColor(Color::Red);
            window.draw(heart);
        }
        else {
            // Empty heart
            CircleShape heart(10.f);
            heart.setPosition(x, y);
            heart.setFillColor(Color(80, 80, 80));
            heart.setOutlineColor(Color::Red);
            heart.setOutlineThickness(2.f);
            window.draw(heart);
        }
    }

    // Draw current weapon name
    if (this->weaponName != nullptr) {
        Text weaponText("Weapon: " + std::string(this->weaponName), this->font, 20);
        weaponText.setPosition(10, 70);
        weaponText.setFillColor(Color(200, 200, 255));
        window.draw(weaponText);
    }
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

void HUD::setWeaponName(const char* name) {
    this->weaponName = name;
}
