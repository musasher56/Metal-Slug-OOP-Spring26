#pragma once
#include "Constants.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class PlayerSoldier;
class Level;
class TextureManager;
class AudioManager;
class FusionCompanion;
class ProjectileManager;  // forward-declare so we can store the pointer


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

    // Stored so that whichever character becomes active after a switch
    // immediately has a valid pm pointer and can shoot/throw on the
    // same frame. Without this every non-Marco character would silently
    // no-op on shoot() because pm was only passed to characters[0] at
    // PlayState construction time.
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

    // Distributes the ProjectileManager to every character slot and stores
    // it for future switches. Called once from PlayState after the managers
    // are wired up.
    void setProjectileManager(ProjectileManager* manager);

    // Sets the starting world position for all character slots. Called from
    // PlayState once the level surface row is known, so no character ever
    // starts at the hardcoded (200, 300) constructor default.
    void initAllPositions(sf::Vector2f startPos);
};