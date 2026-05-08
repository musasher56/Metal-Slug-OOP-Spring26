#include "CharSelectState.h"
#include <cstdio>

// Character names in slot order: 1=Marco, 2=Eri, 3=Tarma, 4=Fio
// (Order matches the uploaded Characterselect.png image)
const char* CharSelectState::CHAR_NAMES[4] = { "Marco", "Eri", "Tarma", "Fio" };

CharSelectState::CharSelectState(TextureManager* texMgr, AudioManager* audMgr)
    : texManager(texMgr)
    , audManager(audMgr)
    , bgLoaded(false)
    , fontLoaded(false)
    , hoveredChar(0)
    , selectedChar(-1)   // -1 = not yet chosen
{
    this->id = GSTATE_CHAR_SELECT;

    // ── Background image ─────────────────────────────────────────────────────
    // Try to load the character select art. If it fails we fall back to a plain
    // dark background with text only — the game won't crash.
    // YOU MUST place the image at:  resources/Sprites/CharacterSelect.png
    if (this->bgTexture.loadFromFile("resources/Sprites/CharacterSelect.png")) {
        this->bgLoaded = true;

        // Scale the image to fill the screen (1280×720) regardless of its
        // original resolution. This is a UI screen so pixelation is acceptable.
        sf::Vector2u texSize = this->bgTexture.getSize();
        float scaleX = static_cast<float>(SCREEN_W) / static_cast<float>(texSize.x);
        float scaleY = static_cast<float>(SCREEN_H) / static_cast<float>(texSize.y);
        this->bgSprite.setTexture(this->bgTexture);
        this->bgSprite.setScale(scaleX, scaleY);
        this->bgSprite.setPosition(0.f, 0.f);
    }

    // ── Font ─────────────────────────────────────────────────────────────────
    // Try system fonts in priority order; same fallback chain as PlayState.
    fontLoaded = this->font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
    if (!fontLoaded) fontLoaded = this->font.loadFromFile("resources/font.ttf");

    // ── Highlight box ─────────────────────────────────────────────────────────
    // A semi-transparent yellow rectangle drawn over whichever slot is hovered.
    // The size matches one portrait panel (~300×480 at 1280×720 scale).
    this->highlightBox.setSize(sf::Vector2f(290.f, 460.f));
    this->highlightBox.setFillColor(sf::Color(255, 255, 0, 50));
    this->highlightBox.setOutlineColor(sf::Color(255, 215, 0));  // gold
    this->highlightBox.setOutlineThickness(4.f);

    // Build the four slot rectangles matching the image layout
    this->buildSlotPositions();
}

CharSelectState::~CharSelectState() {}

void CharSelectState::buildSlotPositions() {
    // The image has 4 equal-width portrait panels side by side.
    // At 1280×720, each panel is ~320px wide. Left edge starts at x~20.
    // These are approximate — tune them to match your image if needed.
    const float panelW  = 296.f;
    const float panelH  = 455.f;
    const float startX  = 22.f;
    const float startY  = 130.f;  // below the "SOLDIER SELECT" header
    const float gapX    = 10.f;   // horizontal gap between panels

    for (int i = 0; i < 4; i++) {
        float x = startX + i * (panelW + gapX);
        this->slotRects[i] = sf::FloatRect(x, startY, panelW, panelH);
    }
}

void CharSelectState::update(float dt) {
    (void)dt;
    // Nothing to animate in this state currently.
    // Highlight position is updated in handleEvent().
}

void CharSelectState::render(RenderWindow& window) {
    // ── Background ────────────────────────────────────────────────────────────
    if (this->bgLoaded) {
        window.draw(this->bgSprite);
    } else {
        // Fallback: dark background if image is missing
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

            // Draw name placeholders for each slot
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
                nameText.setString(CharSelectState::CHAR_NAMES[i]);
                nameText.setCharacterSize(28);
                nameText.setFillColor(sf::Color::White);
                sf::FloatRect nb = nameText.getLocalBounds();
                nameText.setOrigin(nb.width / 2.f, nb.height / 2.f);
                nameText.setPosition(
                    this->slotRects[i].left + this->slotRects[i].width / 2.f,
                    this->slotRects[i].top  + this->slotRects[i].height / 2.f
                );
                window.draw(nameText);
            }
        }
    }

    // ── Highlight box over hovered slot ──────────────────────────────────────
    sf::FloatRect& hovered = this->slotRects[this->hoveredChar];
    this->highlightBox.setPosition(hovered.left, hovered.top);
    window.draw(this->highlightBox);

    // ── Bottom instructions ───────────────────────────────────────────────────
    if (this->fontLoaded) {
        // Slot number labels (1, 2, 3, 4) above each panel — drawn on top of image
        for (int i = 0; i < 4; i++) {
            char numBuf[4];
            // Safe: itoa-equivalent without sprintf for single digit
            numBuf[0] = '1' + i;
            numBuf[1] = '\0';

            sf::Text numText;
            numText.setFont(this->font);
            numText.setString(numBuf);
            numText.setCharacterSize(28);
            numText.setFillColor(i == this->hoveredChar
                ? sf::Color(255, 255, 0)    // yellow when hovered
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

        // Bottom hint bar
        sf::Text hint;
        hint.setFont(this->font);
        hint.setString("[LEFT/RIGHT] Browse  [1/2/3/4] Select  [ENTER] Confirm");
        hint.setCharacterSize(22);
        hint.setFillColor(sf::Color(180, 180, 180));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setOrigin(hb.width / 2.f, 0.f);
        hint.setPosition((float)SCREEN_W / 2.f, (float)SCREEN_H - 42.f);
        window.draw(hint);
    }
}

void CharSelectState::handleEvent(Event& event) {
    if (event.type != Event::KeyPressed) return;

    // Arrow keys cycle through the 4 slots
    if (event.key.code == Keyboard::Left) {
        this->hoveredChar = (this->hoveredChar + 3) % 4;  // wrap left
    }
    else if (event.key.code == Keyboard::Right) {
        this->hoveredChar = (this->hoveredChar + 1) % 4;  // wrap right
    }

    // Direct selection via number keys 1-4
    else if (event.key.code == Keyboard::Num1) { this->hoveredChar = 0; this->selectedChar = 0; }
    else if (event.key.code == Keyboard::Num2) { this->hoveredChar = 1; this->selectedChar = 1; }
    else if (event.key.code == Keyboard::Num3) { this->hoveredChar = 2; this->selectedChar = 2; }
    else if (event.key.code == Keyboard::Num4) { this->hoveredChar = 3; this->selectedChar = 3; }

    // Enter confirms whichever slot is currently hovered
    else if (event.key.code == Keyboard::Return || event.key.code == Keyboard::Space) {
        this->selectedChar = this->hoveredChar;
    }
}

void CharSelectState::onEnter() {
    // Reset selection each time this state is entered so re-opening it
    // (if the state is ever reused) starts fresh.
    this->selectedChar = -1;
    this->hoveredChar  = 0;
}

void CharSelectState::onExit() {}

int CharSelectState::getSelectedChar() const {
    return this->selectedChar;
}