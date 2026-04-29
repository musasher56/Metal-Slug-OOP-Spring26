#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class Level;
class TextureManager;

// WHY: LevelManager owns and manages the current Level
class LevelManager {
private:
    Level* level;
    int currentLevel;

public:
    LevelManager();
    virtual ~LevelManager();
    
    void update(float dt);
    void draw(RenderWindow& window);
    Level* getLevel();
    void nextLevel(TextureManager* tex);
    void saveData();
    void loadData();
};
