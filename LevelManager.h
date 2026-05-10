#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class Level;
class TextureManager;


class LevelManager {
private:
    Level* level;
    int currentLevel;

public:
    LevelManager();
    virtual ~LevelManager();
    
    void update(float dt);
    void draw(RenderWindow& window, float scrollX, float scrollY);
    Level* getLevel();
    void nextLevel(TextureManager* tex);
    void saveData();
    void loadData();
};
