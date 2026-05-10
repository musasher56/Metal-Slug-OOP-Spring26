#include "MainMenu.h"
#include <cstdio>

static const char* LEVEL_NAMES[4] = {
    "Level 1 - Ruins",
    "Level 2 - Cold Death",
    "Level 3 - Blasphemous City",
    "Level 4 - Ironokava"
};

static const char* LEVEL_DESCS[4] = {
    "Mountain + Water",
    "Mountain + Water",
    "Flat Plains - 11km",
    "BOSS ARENA"
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
    , totalVideoFrames(0)
    , currentFrame(0)
    , splashDuration(3.0f)
    , videoLoaded(false)
{
    const char* opts[] = { "SURVIVAL MODE", "CAMPAIGN MODE", "SELF-PLAY (AI)", "EXIT" };
    for (int i = 0; i < 4; i++) {
        int j = 0;
        while (opts[i][j] != '\0' && j < MAX_NAME_LEN - 1) {
            this->options[i][j] = opts[i][j];
            j++;
        }
        this->options[i][j] = '\0';
    }

    this->fontLoaded = this->font.loadFromFile("C:/Windows/Fonts/segoeui.ttf");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!this->fontLoaded)
        this->fontLoaded = this->font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    this->overlay.setSize(Vector2f((float)SCREEN_W, (float)SCREEN_H));
    this->overlay.setFillColor(Color(0, 0, 0, 160));

    this->selector.setSize(Vector2f(500.f, 50.f));
    this->selector.setFillColor(Color(220, 80, 0, 180));

    // Level select boxes
    float boxW = 250.f;
    float boxH = 360.f;
    float gap = 30.f;
    float totalW = 4.f * boxW + 3.f * gap;
    float startX = ((float)SCREEN_W - totalW) / 2.f;
    float boxY = 200.f;

    for (int i = 0; i < 4; i++) {
        float x = startX + i * (boxW + gap);
        this->levelBoxes[i].setSize(Vector2f(boxW, boxH));
        this->levelBoxes[i].setPosition(x, boxY);
        this->levelBoxes[i].setOutlineThickness(3.f);
        // Boss level gets special styling
        if (i == 3) {
            this->levelBoxes[i].setFillColor(Color(50, 20, 20, 180));
            this->levelBoxes[i].setOutlineColor(Color(150, 60, 60));
        }
        else {
            this->levelBoxes[i].setFillColor(Color(20, 20, 50, 180));
            this->levelBoxes[i].setOutlineColor(Color(80, 80, 120));
        }
    }

    this->loadVideoFrames();
}

MainMenu::~MainMenu() {}

bool MainMenu::isReady() const {
    // Ready when a mode AND a level have both been selected.
    // For CAMPAIGN mode, the level is auto-selected (0) — no level select needed.
    // For SURVIVAL/SELF-PLAY, the user must pick a level from the 4 options.
    return (this->gameMode >= 0 && this->gameMode <= 2) && (this->selectedLevel >= 0 && this->selectedLevel <= 3);
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
        printf("[INFO] Loaded %d video frames\n", this->totalVideoFrames);
    }
    else {
        printf("[INFO] No video frames found (optional)\n");
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

    // ── Splash screen ── any key advances to mode select
    if (this->menuState == 0) {
        this->menuState = 1;
        return -1;
    }

    // ── Mode select screen ──
    if (this->menuState == 1) {
        if (event.key.code == Keyboard::Up)
            this->selectedOption = (this->selectedOption - 1 + 4) % 4;
        else if (event.key.code == Keyboard::Down)
            this->selectedOption = (this->selectedOption + 1) % 4;
        else if (event.key.code == Keyboard::Return || event.key.code == Keyboard::Space) {
            if (this->selectedOption == 3) return 99;  // EXIT
            this->gameMode = this->selectedOption;

            if (this->gameMode == MODE_CAMPAIGN) {
                // ── Campaign mode: only ONE Perlin level — auto-select it ──
                // Skip the level select screen entirely since campaign has
                // a single procedural Perlin terrain level (no predefined levels).
                this->selectedLevel = 0;
                // isReady() is now true → MenuState will transition to CharSelect
            }
            else {
                // Survival / Self-Play: show level select with 4 predefined levels
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

    // ── Level select screen ── (only reached for SURVIVAL / SELF-PLAY)
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
            // Go back to mode select
            this->menuState = 1;
            this->selectedLevel = -1;
        }
        return -1;
    }

    return -1;
}

void MainMenu::draw(RenderWindow& window) {
    if (this->menuState == 0) this->drawSplash(window);
    else if (this->menuState == 1) this->drawMain(window);
    else if (this->menuState == 2) this->drawLevelSelect(window);
}

void MainMenu::drawSplash(RenderWindow& window) {
    if (this->videoLoaded)
        window.draw(this->videoSprite);
}

void MainMenu::drawMain(RenderWindow& window) {
    if (this->videoLoaded) window.draw(this->videoSprite);
    window.draw(this->overlay);

    if (!this->fontLoaded) return;

    Text title;
    title.setFont(this->font);
    title.setString("METAL SLUG");
    title.setCharacterSize(64);
    title.setFillColor(Color(255, 100, 0));
    title.setStyle(Text::Bold);
    FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, 0.f);
    title.setPosition(SCREEN_W / 2.f, 50.f);
    window.draw(title);

    const float startY = 250.f;
    const float stepY = 90.f;
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

    Text hint;
    hint.setFont(this->font);
    hint.setString("[UP/DOWN] Navigate  [ENTER] Select  [ESC] Exit");
    hint.setCharacterSize(20);
    hint.setFillColor(Color(160, 160, 160));
    FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2.f, 0.f);
    hint.setPosition(centerX, SCREEN_H - 50.f);
    window.draw(hint);
}

void MainMenu::drawLevelSelect(RenderWindow& window) {
    if (this->videoLoaded) window.draw(this->videoSprite);
    window.draw(this->overlay);

    if (!this->fontLoaded) return;

    // Title
    Text title;
    title.setFont(this->font);
    title.setString("SELECT LEVEL");
    title.setCharacterSize(52);
    title.setFillColor(Color(255, 215, 0));
    title.setStyle(Text::Bold);
    FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, 0.f);
    title.setPosition((float)SCREEN_W / 2.f, 30.f);
    window.draw(title);

    // Mode subtitle
    const char* modeName = "SURVIVAL";
    if (this->gameMode == MODE_SELF_PLAY) modeName = "SELF-PLAY";

    Text modeText;
    modeText.setFont(this->font);
    modeText.setString(modeName);
    modeText.setCharacterSize(24);
    modeText.setFillColor(Color(180, 180, 200));
    FloatRect mb = modeText.getLocalBounds();
    modeText.setOrigin(mb.width / 2.f, 0.f);
    modeText.setPosition((float)SCREEN_W / 2.f, 95.f);
    window.draw(modeText);

    // Level boxes (only survival/self-play reach this screen)
    for (int i = 0; i < 4; i++) {
        // Update colors based on hover
        if (i == this->hoveredLevel) {
            if (i == 3) {
                // Boss level hover: bright red highlight
                this->levelBoxes[i].setFillColor(Color(80, 30, 30, 200));
                this->levelBoxes[i].setOutlineColor(Color(255, 100, 50));
            }
            else {
                this->levelBoxes[i].setFillColor(Color(40, 60, 120, 200));
                this->levelBoxes[i].setOutlineColor(Color(255, 215, 0));
            }
        }
        else {
            if (i == 3) {
                // Boss level default: dark red
                this->levelBoxes[i].setFillColor(Color(50, 20, 20, 180));
                this->levelBoxes[i].setOutlineColor(Color(150, 60, 60));
            }
            else {
                this->levelBoxes[i].setFillColor(Color(20, 20, 50, 180));
                this->levelBoxes[i].setOutlineColor(Color(80, 80, 120));
            }
        }

        window.draw(this->levelBoxes[i]);

        float bx = this->levelBoxes[i].getPosition().x;
        float by = this->levelBoxes[i].getPosition().y;
        float bw = this->levelBoxes[i].getSize().x;
        float bh = this->levelBoxes[i].getSize().y;

        // Big level number
        Text numText;
        numText.setFont(this->font);
        char buf[4];
        buf[0] = '1' + i;
        buf[1] = '\0';
        numText.setString(buf);
        numText.setCharacterSize(80);
        numText.setFillColor(i == this->hoveredLevel
            ? (i == 3 ? Color(255, 100, 50, 200) : Color(255, 215, 0, 200))
            : (i == 3 ? Color(150, 60, 60, 120) : Color(100, 140, 200, 120)));
        FloatRect nr = numText.getLocalBounds();
        numText.setOrigin(nr.width / 2.f, 0.f);
        numText.setPosition(bx + bw / 2.f, by + 30.f);
        window.draw(numText);

        // Level name
        Text nameText;
        nameText.setFont(this->font);
        nameText.setString(LEVEL_NAMES[i]);
        nameText.setCharacterSize(16);
        nameText.setStyle(i == 3 ? Text::Bold : Text::Regular);
        nameText.setFillColor(i == this->hoveredLevel
            ? (i == 3 ? Color(255, 180, 130) : Color(255, 255, 255))
            : (i == 3 ? Color(180, 100, 100) : Color(160, 160, 180)));
        FloatRect nm = nameText.getLocalBounds();
        nameText.setOrigin(nm.width / 2.f, 0.f);
        nameText.setPosition(bx + bw / 2.f, by + 140.f);
        window.draw(nameText);

        // Level description
        Text descText;
        descText.setFont(this->font);
        descText.setString(LEVEL_DESCS[i]);
        descText.setCharacterSize(15);
        descText.setFillColor(i == 3 ? Color(200, 120, 80) : Color(130, 130, 160));
        FloatRect dr = descText.getLocalBounds();
        descText.setOrigin(dr.width / 2.f, 0.f);
        descText.setPosition(bx + bw / 2.f, by + 175.f);
        window.draw(descText);

        // Key hint
        Text keyText;
        keyText.setFont(this->font);
        char keyBuf[8];
        keyBuf[0] = '1' + i;
        keyBuf[1] = '\0';
        keyText.setString(keyBuf);
        keyText.setCharacterSize(26);
        keyText.setFillColor(Color(200, 200, 200, 150));
        FloatRect kr = keyText.getLocalBounds();
        keyText.setOrigin(kr.width / 2.f, 0.f);
        keyText.setPosition(bx + bw / 2.f, by + bh - 50.f);
        window.draw(keyText);

        // Boss skull indicator for level 4
        if (i == 3) {
            Text skullText;
            skullText.setFont(this->font);
            skullText.setString("!! BOSS !!");
            skullText.setCharacterSize(18);
            skullText.setStyle(Text::Bold);
            skullText.setFillColor(i == this->hoveredLevel
                ? Color(255, 60, 30) : Color(180, 50, 30));
            FloatRect sr = skullText.getLocalBounds();
            skullText.setOrigin(sr.width / 2.f, 0.f);
            skullText.setPosition(bx + bw / 2.f, by + 210.f);
            window.draw(skullText);
        }
    }

    // Instructions
    Text hint;
    hint.setFont(this->font);
    hint.setString("[LEFT/RIGHT] Browse   [1/2/3/4] Quick Select   [ENTER] Confirm   [ESC] Back");
    hint.setCharacterSize(20);
    hint.setFillColor(Color(150, 150, 150));
    FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width / 2.f, 0.f);
    hint.setPosition((float)SCREEN_W / 2.f, (float)SCREEN_H - 50.f);
    window.draw(hint);
}
