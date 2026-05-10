#include "CharSelectState.h"
#include "PlayState.h"
#include <cstdio>

const char* CharSelectState::CHAR_NAMES[4] = { "Marco", "Eri", "Tarma", "Fio" };

CharSelectState::CharSelectState(TextureManager* texMgr, AudioManager* audMgr)
    : texManager(texMgr)
    , audManager(audMgr)
    , bgLoaded(false)
    , fontLoaded(false)
    , hoveredChar(0)
{
    this->id = GSTATE_CHAR_SELECT;
    this->shouldGoBack = false;
    this->shouldExit = false;
    // selectedChar, gameMode, selectedLevel are inherited from GameState
    // and already set by MenuState before this object was created.
    // selectedChar is -1 until player confirms.

    if (this->bgTexture.loadFromFile("resources/Sprites/CharacterSelect.png")) {
        this->bgLoaded = true;
        sf::Vector2u texSize = this->bgTexture.getSize();
        float scaleX = static_cast<float>(SCREEN_W) / static_cast<float>(texSize.x);
        float scaleY = static_cast<float>(SCREEN_H) / static_cast<float>(texSize.y);
        this->bgSprite.setTexture(this->bgTexture);
        this->bgSprite.setScale(scaleX, scaleY);
        this->bgSprite.setPosition(0.f, 0.f);
    }

    fontLoaded = this->font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    this->highlightBox.setSize(sf::Vector2f(290.f, 460.f));
    this->highlightBox.setFillColor(sf::Color(255, 255, 0, 50));
    this->highlightBox.setOutlineColor(sf::Color(255, 215, 0));
    this->highlightBox.setOutlineThickness(4.f);

    this->buildSlotPositions();
}

CharSelectState::~CharSelectState() {}

void CharSelectState::buildSlotPositions() {
    const float panelW = 296.f;
    const float panelH = 455.f;
    const float startX = 22.f;
    const float startY = 130.f;
    const float gapX = 10.f;

    for (int i = 0; i < 4; i++) {
        float x = startX + i * (panelW + gapX);
        this->slotRects[i] = sf::FloatRect(x, startY, panelW, panelH);
    }
}

void CharSelectState::update(float dt) {
    (void)dt;
}

void CharSelectState::render(RenderWindow& window) {
    if (this->bgLoaded) {
        window.draw(this->bgSprite);
    }
    else {
        sf::RectangleShape bg(sf::Vector2f((float)SCREEN_W, (float)SCREEN_H));
        bg.setFillColor(sf::Color(20, 20, 20));
        window.draw(bg);

        if (this->fontLoaded) {
            sf::Text title;
            title.setFont(this->font);
            title.setString("SOLDIER SELECT");
            title.setCharacterSize(64);
            title.setFillColor(sf::Color(255, 80, 0));
            title.setStyle(sf::Text::Bold);
            sf::FloatRect tb = title.getLocalBounds();
            title.setOrigin(tb.width / 2.f, 0.f);
            title.setPosition((float)SCREEN_W / 2.f, 40.f);
            window.draw(title);

            for (int i = 0; i < 4; i++) {
                sf::RectangleShape panel(sf::Vector2f(this->slotRects[i].width,
                    this->slotRects[i].height));
                panel.setPosition(this->slotRects[i].left, this->slotRects[i].top);
                panel.setFillColor(sf::Color(40, 40, 40));
                panel.setOutlineColor(sf::Color(100, 100, 100));
                panel.setOutlineThickness(2.f);
                window.draw(panel);

                sf::Text nameText;
                nameText.setFont(this->font);
                nameText.setString(CHAR_NAMES[i]);
                nameText.setCharacterSize(28);
                nameText.setFillColor(sf::Color::White);
                sf::FloatRect nb = nameText.getLocalBounds();
                nameText.setOrigin(nb.width / 2.f, nb.height / 2.f);
                nameText.setPosition(
                    this->slotRects[i].left + this->slotRects[i].width / 2.f,
                    this->slotRects[i].top + this->slotRects[i].height / 2.f
                );
                window.draw(nameText);
            }
        }
    }

    sf::FloatRect& hovered = this->slotRects[this->hoveredChar];
    this->highlightBox.setPosition(hovered.left, hovered.top);
    window.draw(this->highlightBox);

    if (this->fontLoaded) {
        for (int i = 0; i < 4; i++) {
            char numBuf[4];
            numBuf[0] = '1' + i;
            numBuf[1] = '\0';

            sf::Text numText;
            numText.setFont(this->font);
            numText.setString(numBuf);
            numText.setCharacterSize(28);
            numText.setFillColor(i == this->hoveredChar
                ? sf::Color(255, 255, 0)
                : sf::Color(200, 200, 200));
            numText.setStyle(sf::Text::Bold);
            sf::FloatRect nb = numText.getLocalBounds();
            numText.setOrigin(nb.width / 2.f, nb.height / 2.f);
            numText.setPosition(
                this->slotRects[i].left + this->slotRects[i].width / 2.f,
                this->slotRects[i].top + this->slotRects[i].height + 18.f
            );
            window.draw(numText);
        }

        sf::Text hint;
        hint.setFont(this->font);
        hint.setString("[LEFT/RIGHT] Browse  [1/2/3/4] Select  [ENTER] Confirm  [ESC] Back");
        hint.setCharacterSize(22);
        hint.setFillColor(sf::Color(180, 180, 180));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2.f, 0.f);
        hint.setPosition((float)SCREEN_W / 2.f, (float)SCREEN_H - 42.f);
        window.draw(hint);
    }
}

void CharSelectState::handleEvent(sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::Left) {
        this->hoveredChar = (this->hoveredChar + 3) % 4;
    }
    else if (event.key.code == sf::Keyboard::Right) {
        this->hoveredChar = (this->hoveredChar + 1) % 4;
    }
    else if (event.key.code == sf::Keyboard::Num1) { this->hoveredChar = 0; this->selectedChar = 0; }
    else if (event.key.code == sf::Keyboard::Num2) { this->hoveredChar = 1; this->selectedChar = 1; }
    else if (event.key.code == sf::Keyboard::Num3) { this->hoveredChar = 2; this->selectedChar = 2; }
    else if (event.key.code == sf::Keyboard::Num4) { this->hoveredChar = 3; this->selectedChar = 3; }
    else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
        this->selectedChar = this->hoveredChar;
    }
    else if (event.key.code == sf::Keyboard::Escape) {
        this->shouldGoBack = true;
    }
}

void CharSelectState::onEnter() {
    this->selectedChar = -1;
    this->hoveredChar = 0;
    this->shouldGoBack = false;

    // Continue playing title theme during character selection.
    // If the music was stopped (e.g. returning from PlayState), restart it.
    if (this->audManager != nullptr && !this->audManager->isMusicPlaying()) {
        this->audManager->playMusicTrack(0);  // Track 0 = title_theme.ogg
    }
}

void CharSelectState::onExit() {
    // Stop title music when leaving char select (entering gameplay or going back).
    if (this->audManager != nullptr) {
        this->audManager->stopMusic();
    }
}

GameState* CharSelectState::createNextState() {
    // Only transition when a character has been confirmed
    if (this->selectedChar >= 0 && this->selectedChar <= 3) {
        PlayState* play = new PlayState(this->gameMode,
            this->selectedChar,
            this->texManager,
            this->audManager,
            this->selectedLevel);
        play->setStateManager(this->stateManager);
        return play;
    }
    return nullptr;
}
