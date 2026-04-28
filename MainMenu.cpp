#include "MainMenu.h"
#include <cstdio>
//test
MainMenu::MainMenu(TextureManager* tex, AudioManager* aud)
    : selectedOption(0)
    , gameMode(MODE_SURVIVAL)
    , menuState(0)
    , texManager(tex)
    , audManager(aud)
    , fontLoaded(false)
    , totalVideoFrames(0)
    , currentFrame(0)
    , splashDuration(3.0f)
    , videoLoaded(false)
{
    const char* opts[] = {"SURVIVAL MODE", "CAMPAIGN MODE", "SELF-PLAY (AI)", "EXIT"};
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

    this->loadVideoFrames();
}

MainMenu::~MainMenu() {}

void MainMenu::loadVideoFrames() {
    char path[128];
    this->totalVideoFrames = 0;

    for (int i = 1; i <= 270 && this->totalVideoFrames < 270; i++) {
        int pos = 0;
        const char* prefix = "resources/menu_frames/frame_";
        for (int k = 0; prefix[k] != '\0' && pos < 120; k++)
            path[pos++] = prefix[k];

        path[pos++] = '0' + (i / 1000) % 10;
        path[pos++] = '0' + (i / 100)  % 10;
        path[pos++] = '0' + (i / 10)   % 10;
        path[pos++] = '0' + (i)        % 10;

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
    } else {
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

    if (this->menuState == 0) {
        this->menuState = 1;
        return -1;
    }

    if (event.key.code == Keyboard::Up)
        this->selectedOption = (this->selectedOption - 1 + 4) % 4;
    else if (event.key.code == Keyboard::Down)
        this->selectedOption = (this->selectedOption + 1) % 4;
    else if (event.key.code == Keyboard::Return || event.key.code == Keyboard::Space) {
        if (this->selectedOption == 3) return 99;
        this->gameMode = this->selectedOption;
        return this->gameMode;
    } else if (event.key.code == Keyboard::Escape)
        return 99;

    return -1;
}

void MainMenu::draw(RenderWindow& window) {
    if (this->menuState == 0) this->drawSplash(window);
    else                      this->drawMain(window);
}

void MainMenu::drawSplash(RenderWindow& window) {
    // WHY: pure fullscreen video — zero text, zero overlay
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

    const float startY  = 250.f;
    const float stepY   = 90.f;
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