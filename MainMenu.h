#pragma once
#include "Constants.h"
#include "TextureManager.h"
#include "AudioManager.h"

class MainMenu {
public:
    MainMenu(TextureManager* tex, AudioManager* aud);
    ~MainMenu();

    int handleEvent(Event& event);
    void draw(RenderWindow& window);
    void update(float dt);

    int getSelectedMode() const { return gameMode; }
    int getSelectedLevel() const { return selectedLevel; }
    bool isReady() const;          // true when both mode and level are chosen

private:
    char options[4][MAX_NAME_LEN];
    int selectedOption;
    int gameMode;
    int selectedLevel;
    int menuState;   // 0=splash, 1=mode select, 2=level select

    TextureManager* texManager;
    AudioManager* audManager;

    Font font;
    bool fontLoaded;

    RectangleShape overlay;
    RectangleShape selector;

    // Level select highlight box (like character select)
    RectangleShape levelHighlightBox;
    FloatRect levelSlotRects[4];
    int hoveredLevel;

    // Title screen background
    Texture titleScreenTex;
    Sprite  titleScreenSprite;
    bool    titleScreenLoaded;

    // Level select background
    Texture levelSelectBgTex;
    Sprite  levelSelectBgSprite;
    bool    levelSelectBgLoaded;

    Clock splashTimer;
    Clock videoTimer;
    float splashDuration;

    Texture videoFrames[270];
    int totalVideoFrames;
    int currentFrame;
    Sprite videoSprite;
    bool videoLoaded;

    void loadVideoFrames();
    void updateVideo(float dt);
    void drawSplash(RenderWindow& window);
    void drawMain(RenderWindow& window);
    void drawLevelSelect(RenderWindow& window);

    void buildLevelSlotPositions();
};
