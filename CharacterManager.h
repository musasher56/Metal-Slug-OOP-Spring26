#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class PlayerSoldier;
class Level;
class TextureManager;
class AudioManager;
class FusionCompanion;
class ProjectileManager;

class CharacterManager {
    private:
    PlayerSoldier* characters[4];
    int currentCharacter;
    int kills;
    int rings;
    Clock switchTimer;
    Clock invincibilityTimer;
    FusionCompanion* fusionCompanion;
    bool fusionAvailable;
    Clock fusionCooldown;
    TextureManager* texManager;
    AudioManager* audManager;
    ProjectileManager* pm;

public:
    CharacterManager(TextureManager* texMgr, AudioManager* audMgr, int startChar = 0);
    virtual ~CharacterManager();
    void update(float dt, Level* lvl);
    void draw(RenderWindow& window, float scrollX, float scrollY);
    void handleInput(Event& event);
    void switchCharacter();
    void takeDamage(int amount);
    PlayerSoldier* getCurrentCharacter();
    int getCurrentCharacterIdx();
    int getHealthPoints();
    int getRings();
    int getKills();
    void incKills();
    void spawnFusion();
    bool isFusionActive();
    bool anyCharacterAlive();
    void saveData(float scroll);
    void loadData(float scroll);
    void setProjectileManager(ProjectileManager* manager);
    void initAllPositions(sf::Vector2f startPos);
};