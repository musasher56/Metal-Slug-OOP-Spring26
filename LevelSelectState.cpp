#include "LevelSelectState.h"
#include "GameStateManager.h"
#include "PlayState.h"
#include <cstdio>

const char* LevelSelectState::LEVEL_NAMES[3] = {
    "Level 1 - Grassland",
    "Level 2 - Desert",
    "Level 3 - Plains"
};

LevelSelectState::LevelSelectState(TextureManager* texMgr, AudioManager* audMgr, int score)
    : texManager(texMgr)
    , audManager(audMgr)
    , stateManager(nullptr)
    , finalScore(score)
    , selectedLevel(0)
    , maxUnlockedLevel(2)   // all levels unlocked after campaign
{
    this->id = GSTATE_LEVEL_SELECT;

    // Try loading fonts
    bool fontLoaded = this->menuFont.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->menuFont.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->menuFont.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
    if (!fontLoaded) fontLoaded = this->menuFont.loadFromFile("resources/font.ttf");

    // Background overlay
    this->bgOverlay.setSize(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
    this->bgOverlay.setFillColor(sf::Color(10, 10, 30, 230));
    this->bgOverlay.setPosition(0.f, 0.f);

    // Setup all text elements
    this->setupText();
}

LevelSelectState::~LevelSelectState() {
}

void LevelSelectState::setupText() {
    // Title
    this->titleText.setFont(this->menuFont);
    this->titleText.setString("CAMPAIGN COMPLETE!");
    this->titleText.setCharacterSize(48);
    this->titleText.setFillColor(sf::Color(255, 215, 0));  // gold
    this->titleText.setStyle(sf::Text::Bold);
    this->titleText.setPosition(
        (float)SCREEN_W / 2.f - this->titleText.getLocalBounds().width / 2.f,
        40.f
    );

    // Score text
    char scoreBuf[64];
    sprintf(scoreBuf, "Final Score: %d", this->finalScore);
    this->scoreText.setFont(this->menuFont);
    this->scoreText.setString(scoreBuf);
    this->scoreText.setCharacterSize(28);
    this->scoreText.setFillColor(sf::Color(200, 200, 200));
    this->scoreText.setPosition(
        (float)SCREEN_W / 2.f - this->scoreText.getLocalBounds().width / 2.f,
        110.f
    );

    // Level boxes and texts
    float boxW = 220.f;
    float boxH = 180.f;
    float gap = 30.f;
    float totalW = 3.f * boxW + 2.f * gap;
    float startX = ((float)SCREEN_W - totalW) / 2.f;
    float boxY = 190.f;

    for (int i = 0; i < 3; i++) {
        float x = startX + i * (boxW + gap);

        // Box
        this->levelBoxes[i].setSize(sf::Vector2f(boxW, boxH));
        this->levelBoxes[i].setPosition(x, boxY);
        this->levelBoxes[i].setOutlineThickness(3.f);

        // Level text
        this->levelTexts[i].setFont(this->menuFont);
        this->levelTexts[i].setString(LEVEL_NAMES[i]);
        this->levelTexts[i].setCharacterSize(20);
        this->levelTexts[i].setStyle(sf::Text::Bold);

        // Level number
        char numBuf[8];
        sprintf(numBuf, "%d", i + 1);

        // Position text centered in box
        sf::FloatRect textRect = this->levelTexts[i].getLocalBounds();
        this->levelTexts[i].setPosition(
            x + (boxW - textRect.width) / 2.f,
            boxY + boxH / 2.f - 10.f
        );
    }

    // Instruction text
    this->instructionText.setFont(this->menuFont);
    this->instructionText.setString("Arrow Keys = Select    ENTER = Play    ESC = Quit");
    this->instructionText.setCharacterSize(20);
    this->instructionText.setFillColor(sf::Color(150, 150, 150));
    this->instructionText.setPosition(
        (float)SCREEN_W / 2.f - this->instructionText.getLocalBounds().width / 2.f,
        430.f
    );
}

void LevelSelectState::update(float dt) {
    // Update colors based on selection
    for (int i = 0; i < 3; i++) {
        if (i == this->selectedLevel) {
            this->levelBoxes[i].setFillColor(sf::Color(40, 60, 120, 200));
            this->levelBoxes[i].setOutlineColor(sf::Color(255, 215, 0));  // gold border
            this->levelTexts[i].setFillColor(sf::Color(255, 255, 255));
        }
        else {
            this->levelBoxes[i].setFillColor(sf::Color(20, 20, 50, 180));
            this->levelBoxes[i].setOutlineColor(sf::Color(80, 80, 120));
            this->levelTexts[i].setFillColor(sf::Color(160, 160, 180));
        }
    }
}

void LevelSelectState::render(sf::RenderWindow& window) {
    // Dark background
    window.draw(this->bgOverlay);

    // Title and score
    window.draw(this->titleText);
    window.draw(this->scoreText);

    // Level boxes and labels
    for (int i = 0; i < 3; i++) {
        window.draw(this->levelBoxes[i]);

        // Level number big text
        sf::Text numText;
        numText.setFont(this->menuFont);
        char buf[4];
        sprintf(buf, "%d", i + 1);
        numText.setString(buf);
        numText.setCharacterSize(60);
        numText.setFillColor(sf::Color(100, 140, 200, 120));

        float bx = this->levelBoxes[i].getPosition().x;
        float by = this->levelBoxes[i].getPosition().y;
        float bw = this->levelBoxes[i].getSize().x;
        float bh = this->levelBoxes[i].getSize().y;

        sf::FloatRect nr = numText.getLocalBounds();
        numText.setPosition(
            bx + (bw - nr.width) / 2.f,
            by + 20.f
        );
        window.draw(numText);

        // Level name
        window.draw(this->levelTexts[i]);

        // Level description below name
        sf::Text descText;
        descText.setFont(this->menuFont);
        descText.setCharacterSize(14);
        descText.setFillColor(sf::Color(130, 130, 160));

        if (i == 0) {
            descText.setString("Mountain + Water");
        }
        else if (i == 1) {
            descText.setString("Mountain + Water");
        }
        else {
            descText.setString("Flat Plains - No Water");
        }

        sf::FloatRect dr = descText.getLocalBounds();
        descText.setPosition(
            bx + (bw - dr.width) / 2.f,
            by + bh - 35.f
        );
        window.draw(descText);
    }

    // Instructions
    window.draw(this->instructionText);
}

void LevelSelectState::handleEvent(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        // Navigate left/right
        if (event.key.code == sf::Keyboard::Left) {
            this->selectedLevel--;
            if (this->selectedLevel < 0) this->selectedLevel = 2;
        }
        else if (event.key.code == sf::Keyboard::Right) {
            this->selectedLevel++;
            if (this->selectedLevel > 2) this->selectedLevel = 0;
        }
        else if (event.key.code == sf::Keyboard::Up) {
            this->selectedLevel--;
            if (this->selectedLevel < 0) this->selectedLevel = 2;
        }
        else if (event.key.code == sf::Keyboard::Down) {
            this->selectedLevel++;
            if (this->selectedLevel > 2) this->selectedLevel = 0;
        }
        // Confirm selection
        else if (event.key.code == sf::Keyboard::Return) {
            this->launchSelectedLevel();
        }
        // ESC — quit
        else if (event.key.code == sf::Keyboard::Escape) {
            if (this->stateManager != nullptr) {
                // Could go to a main menu or just quit
                // For now, relaunch level 0 as campaign
                this->selectedLevel = 0;
                this->launchSelectedLevel();
            }
        }
    }

    // Mouse click support
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(
                static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y)
            );

            for (int i = 0; i < 3; i++) {
                sf::FloatRect boxRect(
                    this->levelBoxes[i].getPosition().x,
                    this->levelBoxes[i].getPosition().y,
                    this->levelBoxes[i].getSize().x,
                    this->levelBoxes[i].getSize().y
                );

                if (boxRect.contains(mousePos)) {
                    this->selectedLevel = i;
                    this->launchSelectedLevel();
                    break;
                }
            }
        }
    }
}

void LevelSelectState::launchSelectedLevel() {
    if (this->stateManager == nullptr) return;

    // Create PlayState starting at the selected level
    PlayState* play = new PlayState(0, 0, this->texManager, this->audManager, this->selectedLevel);
    play->setStateManager(this->stateManager);
    this->stateManager->changeState(play);
}

void LevelSelectState::onEnter() {
    // Re-center title in case window resized
    this->titleText.setPosition(
        (float)SCREEN_W / 2.f - this->titleText.getLocalBounds().width / 2.f,
        40.f
    );
}

void LevelSelectState::onExit() {
}
