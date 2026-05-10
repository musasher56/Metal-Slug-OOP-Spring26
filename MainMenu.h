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
    bool isReady() const;          

private:
    char options[4][MAX_NAME_LEN];
    int selectedOption;
    int gameMode;
    int selectedLevel;
    int menuState;   

    TextureManager* texManager;
    AudioManager* audManager;

    Font font;
    bool fontLoaded;

    RectangleShape overlay;
    RectangleShape selector;

    
    RectangleShape levelBoxes[4];
    int hoveredLevel;

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
};
