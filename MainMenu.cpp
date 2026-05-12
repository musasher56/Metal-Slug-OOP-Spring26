#include "MainMenu.h"
#include <cstdio>

static const char* LEVEL_NAMES[4] = {
    "Level 1 - Ruins",
    "Level 2 - Cold Death",
    "Level 3 - Blasphemous City",
    "Level 4 - Ironokava"
};

MainMenu::MainMenu(TextureManager* tex, AudioManager* aud)
    : selectedOption(0)
    , gameMode(MODE_SURVIVAL)
    , selectedLevel(-1)
    , menuState(0)
    , texManager(tex)
    , audManager(aud)
    , fontLoaded(false)
    , hoveredLevel(0)
    , titleScreenLoaded(false)
    , levelSelectBgLoaded(false)
    , totalVideoFrames(0)
    , currentFrame(0)
    , splashDuration(3.0f)
    , videoLoaded(false)
{
    const char* opts[] = { "SURVIVAL MODE", "CAMPAIGN MODE", "SELF-PLAY MODE", "EXIT" };
    for (int i = 0; i < 4; i++) {
        int j = 0;
        while (opts[i][j] != '\0' && j < MAX_NAME_LEN - 1) {
            this->options[i][j] = opts[i][j];
            j++;
        }
        this->options[i][j] = '\0';
    }

    this->fontLoaded = this->font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("/Library/Fonts/Arial.ttf");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    this->selector.setSize(Vector2f(500.f, 50.f));
    this->selector.setFillColor(Color(220, 80, 0, 180));

    if (this->titleScreenTex.loadFromFile("resources/Sprites/Titlescreen.png")) {
        this->titleScreenSprite.setTexture(this->titleScreenTex);
        Vector2u sz = this->titleScreenTex.getSize();
        float sx = (float)SCREEN_W / (float)sz.x;
        float sy = (float)SCREEN_H / (float)sz.y;
        this->titleScreenSprite.setScale(sx, sy);
        this->titleScreenLoaded = true;
    }
    if (this->levelSelectBgTex.loadFromFile("resources/Sprites/LevelSelect.png")) {
        this->levelSelectBgSprite.setTexture(this->levelSelectBgTex);
        Vector2u sz = this->levelSelectBgTex.getSize();
        float sx = (float)SCREEN_W / (float)sz.x;
        float sy = (float)SCREEN_H / (float)sz.y;
        this->levelSelectBgSprite.setScale(sx, sy);
        this->levelSelectBgSprite.setPosition(0.f, 0.f);
        this->levelSelectBgLoaded = true;
    }
    this->levelHighlightBox.setSize(Vector2f(280.f, 400.f));
    this->levelHighlightBox.setFillColor(Color(255, 255, 0, 50));
    this->levelHighlightBox.setOutlineColor(Color(255, 215, 0));
    this->levelHighlightBox.setOutlineThickness(4.f);

    this->buildLevelSlotPositions();

    this->loadVideoFrames();
}

MainMenu::~MainMenu() {}

bool MainMenu::isReady() const {
    return (this->gameMode >= 0 && this->gameMode <= 2) && (this->selectedLevel >= 0 && this->selectedLevel <= 3);
}

void MainMenu::buildLevelSlotPositions() {
    const float panelW = 280.f;
    const float panelH = 400.f;
    const float gapX = 30.f;
    const float totalW = 4.f * panelW + 3.f * gapX;
    const float startX = ((float)SCREEN_W - totalW) / 2.f;
    const float startY = 180.f;

    for (int i = 0; i < 4; i++) {
        float x = startX + i * (panelW + gapX);
        this->levelSlotRects[i] = FloatRect(x, startY, panelW, panelH);
    }
}

void MainMenu::loadVideoFrames() {
    char path[128];
    this->totalVideoFrames = 0;

    for (int i = 1; i <= 270 && this->totalVideoFrames < 270; i++) {
        int pos = 0;
        const char* prefix = "resources/menu_frames/frame_";
        for (int k = 0; prefix[k] != '\0' && pos < 120; k++)
            path[pos++] = prefix[k];

        path[pos++] = '0' + (i / 1000) % 10;
        path[pos++] = '0' + (i / 100) % 10;
        path[pos++] = '0' + (i / 10) % 10;
        path[pos++] = '0' + (i) % 10;

        const char* ext = ".png";
        for (int k = 0; ext[k] != '\0' && pos < 127; k++)
            path[pos++] = ext[k];
        path[pos] = '\0';

        if (!this->videoFrames[this->totalVideoFrames].loadFromFile(path))
            break;
        this->totalVideoFrames++;
    }

    if (this->totalVideoFrames > 0) {
        this->videoSprite.setTexture(this->videoFrames[0]);
        Vector2u sz = this->videoFrames[0].getSize();
        float sx = (float)SCREEN_W / (float)sz.x;
        float sy = (float)SCREEN_H / (float)sz.y;
        this->videoSprite.setScale(sx, sy);
        this->videoLoaded = true;
    }
    else {
    }
}

void MainMenu::update(float dt) {
    if (this->videoLoaded && this->totalVideoFrames > 0)
        this->updateVideo(dt);
}

void MainMenu::updateVideo(float dt) {
    static Clock frameClock;
    if (frameClock.getElapsedTime().asSeconds() >= 1.0f / 24.0f) {
        this->currentFrame = (this->currentFrame + 1) % this->totalVideoFrames;
        this->videoSprite.setTexture(this->videoFrames[this->currentFrame]);
        frameClock.restart();
    }
}

int MainMenu::handleEvent(Event& event) {
    if (event.type != Event::KeyPressed) return -1;

    if (this->menuState == 0) {
        this->menuState = 1;
        return -1;
    }

    if (this->menuState == 1) {
        if (event.key.code == Keyboard::Up)
            this->selectedOption = (this->selectedOption - 1 + 4) % 4;
        else if (event.key.code == Keyboard::Down)
            this->selectedOption = (this->selectedOption + 1) % 4;
        else if (event.key.code == Keyboard::Return || event.key.code == Keyboard::Space) {
            if (this->selectedOption == 3) return 99;  // EXIT
            this->gameMode = this->selectedOption;

            if (this->gameMode == MODE_CAMPAIGN) {
                this->selectedLevel = 0;
            }
            else {
                this->menuState = 2;
                this->selectedLevel = -1;
                this->hoveredLevel = 0;
            }
        }
        else if (event.key.code == Keyboard::Escape) {
            return 99;
        }
        return -1;
    }
    if (this->menuState == 2) {
        if (event.key.code == Keyboard::Left) {
            this->hoveredLevel = (this->hoveredLevel + 3) % 4;
        }
        else if (event.key.code == Keyboard::Right) {
            this->hoveredLevel = (this->hoveredLevel + 1) % 4;
        }
        else if (event.key.code == Keyboard::Num1) {
            this->hoveredLevel = 0;
            this->selectedLevel = 0;
        }
        else if (event.key.code == Keyboard::Num2) {
            this->hoveredLevel = 1;
            this->selectedLevel = 1;
        }
        else if (event.key.code == Keyboard::Num3) {
            this->hoveredLevel = 2;
            this->selectedLevel = 2;
        }
        else if (event.key.code == Keyboard::Num4) {
            this->hoveredLevel = 3;
            this->selectedLevel = 3;
        }
        else if (event.key.code == Keyboard::Return || event.key.code == Keyboard::Space) {
            this->selectedLevel = this->hoveredLevel;
        }
        else if (event.key.code == Keyboard::Escape) {
            this->menuState = 1;
            this->selectedLevel = -1;
        }
        return -1;
    }

    return -1;
}

void MainMenu::draw(RenderWindow& window) {
    if (this->menuState == 0)
        this->drawSplash(window);
    else if (this->menuState == 1)
        this->drawMain(window);
    else if (this->menuState == 2)
        this->drawLevelSelect(window);
}

void MainMenu::drawSplash(RenderWindow& window) {
    if (this->videoLoaded)
        window.draw(this->videoSprite);
}

void MainMenu::drawMain(RenderWindow& window) {

    if (this->titleScreenLoaded) {
        window.draw(this->titleScreenSprite);
    }
    else if (this->videoLoaded) {
        window.draw(this->videoSprite);
    }

    if (!this->fontLoaded)
        return;

    const float startY = 280.f;
    const float stepY = 85.f;
    const float centerX = SCREEN_W / 2.f;

    for (int i = 0; i < 4; i++) {
        float y = startY + (float)i * stepY;

        if (i == this->selectedOption) {
            this->selector.setPosition(centerX - 250.f, y - 10.f);
            window.draw(this->selector);
        }

        Text opt;
        opt.setFont(this->font);
        opt.setString(this->options[i]);
        opt.setCharacterSize(36);
        opt.setStyle(i == this->selectedOption ? Text::Bold : Text::Regular);
        opt.setFillColor(i == this->selectedOption ? Color(255, 255, 100) : Color(220, 220, 220));
        FloatRect b = opt.getLocalBounds();
        opt.setOrigin(b.width / 2.f, b.height / 2.f);
        opt.setPosition(centerX, y);
        window.draw(opt);
    }
}

void MainMenu::drawLevelSelect(RenderWindow& window) {

    if (this->levelSelectBgLoaded) {
        window.draw(this->levelSelectBgSprite);
    }
    else if (this->videoLoaded) {
        window.draw(this->videoSprite);
    }
    else {
      
        RectangleShape bg(Vector2f((float)SCREEN_W, (float)SCREEN_H));
        bg.setFillColor(Color(20, 20, 20));
        window.draw(bg);
    }

    if (!this->fontLoaded)
        return;

    for (int i = 0; i < 4; i++) {
        char numBuf[2];
        numBuf[0] = '1' + i;
        numBuf[1] = '\0';

        Text numText;
        numText.setFont(this->font);
        numText.setString(numBuf);
        numText.setCharacterSize(28);
        numText.setFillColor(i == this->hoveredLevel? Color(255, 255, 0): Color(200, 200, 200));
        numText.setStyle(Text::Bold);
        FloatRect nb = numText.getLocalBounds();
        numText.setOrigin(nb.width / 2.f, nb.height / 2.f);
        numText.setPosition(
            this->levelSlotRects[i].left + this->levelSlotRects[i].width / 2.f,
            this->levelSlotRects[i].top + this->levelSlotRects[i].height + 20.f
        );
        window.draw(numText);
    }

}
