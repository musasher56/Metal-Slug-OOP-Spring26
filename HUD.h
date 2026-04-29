#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class CharacterManager;

// WHY: HUD displays score, HP, and other gameplay info
class HUD {
private:
    int score;
    int hp;
    Clock elapsedTime;
    Font font;
    float redHueAlpha;

public:
    HUD();
    virtual ~HUD();
    
    void update(CharacterManager* cm, int levelNum);
    void draw(RenderWindow& window);
    void setScore(int s);
    int getScore() const;
    void showDamageHue(float intensity);
};
