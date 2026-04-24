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

private:
    char options[4][MAX_NAME_LEN];
    int selectedOption;
    int gameMode;
    int menuState; // 0=splash, 1=main

    TextureManager* texManager;
    AudioManager* audManager;

    Font font;
    bool fontLoaded;

    RectangleShape overlay;
    RectangleShape selector;

    Clock splashTimer;
    Clock videoTimer;
    float splashDuration;

    // Video frames (optional)
    Texture videoFrames[270];
    int totalVideoFrames;
    int currentFrame;
    Sprite videoSprite;
    bool videoLoaded;

    void loadVideoFrames();
    void updateVideo(float dt);
    void drawSplash(RenderWindow& window);
    void drawMain(RenderWindow& window);
};