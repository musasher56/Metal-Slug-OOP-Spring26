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

    // -------------------------------------------------------------------------
    // setLevel: Replaces the current Level with a new one.
    // Used by PlayState when switching to campaign mode — it creates a
    // noise-generated Level(NoiseProfile*) and passes ownership here.
    // LevelManager deletes the old level and takes ownership of the new one.
    // Called BEFORE loadLevel() rebuilds terrain so the grid is correct.
    // -------------------------------------------------------------------------
    void setLevel(Level* newLevel);
};