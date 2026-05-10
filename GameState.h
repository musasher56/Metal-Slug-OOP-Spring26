#pragma once
#include "Constants.h"
#include "Vector2d.h"

class TextureManager;
class AudioManager;
class GameStateManager;

// ============================================================================
//  GameState — base class for all game screens.
//
//  Polymorphic transitions: each state overrides createNextState(). When the
//  state is ready to advance (player made a selection), it returns a new
//  GameState* for the next screen. Game.cpp calls this every frame — no
//  if-chains on state IDs needed.
//
//  Shared context: gameMode / selectedLevel / selectedChar live on the base
//  class so every state can read them and pass them forward when constructing
//  the next state.
//
//  shouldGoBack: set true if the player presses ESC and wants to return to
//  the previous screen. Game.cpp will pop the current state.
//
//  shouldExit: set true if the player wants to quit the entire game.
// ============================================================================

class GameState {
public:
    // Shared context — public so Game.cpp can copy them forward between states
    int gameMode;
    int selectedLevel;    // 0, 1, 2, or 3
    int selectedChar;     // 0=Marco, 1=Tarma, 2=Eri, 3=Fio

    TextureManager* texManager;
    AudioManager* audManager;
    GameStateManager* stateManager;

    bool shouldGoBack;    // ESC was pressed — pop this state
    bool shouldExit;      // player wants to quit the game

protected:
    int id;

public:
    GameState();
    virtual ~GameState();

    virtual void update(float dt) = 0;
    virtual void render(RenderWindow& window) = 0;
    virtual void handleEvent(Event& event) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;

    // Returns the next state to transition to, or nullptr if not ready yet.
    // Game.cpp calls this every frame after handleEvent(). If non-null, the
    // current state is popped and the returned state is pushed.
    virtual GameState* createNextState();

    int getID() const;

    bool getShouldGoBack() const { return this->shouldGoBack; }
    bool getShouldExit() const { return this->shouldExit; }

    void setStateManager(GameStateManager* mgr) { this->stateManager = mgr; }
};
