#include "Game.h"
#include <cstdio>

Game::Game()
    : window(VideoMode(SCREEN_W, SCREEN_H), "Metal Slug - OOP SP26")
    , stateManager(nullptr)
    , texManager(nullptr)
    , audManager(nullptr)
    , gameMode(MODE_SURVIVAL)
    , running(true)
{
    this->window.setFramerateLimit(FRAMERATE_LIMIT);
    this->initialize();
}

Game::~Game() {
    this->cleanup();
}

void Game::initialize() {
    this->texManager = new TextureManager();
    this->audManager = new AudioManager();
    this->stateManager = new GameStateManager();
    
    // WHY: Start with MenuState as the initial game state
    MenuState* menu = new MenuState(this->texManager, this->audManager);
    this->stateManager->push(menu);
}

void Game::run() {
    Clock clock;
    while (this->running && this->window.isOpen()) {
        float dt = clock.restart().asSeconds();
        this->handleEvents();
        this->update(dt);
        this->render();
    }
}

void Game::handleEvents() {
    Event ev;
    while (this->window.pollEvent(ev)) {
        if (ev.type == Event::Closed) {
            this->running = false;
            this->window.close();
            return;
        }

        // WHY: Delegate event handling to current state via GameStateManager
        if (this->stateManager != nullptr) {
            this->stateManager->handleEvent(ev);
            
            // WHY: Check for state changes after event handling
            GameState* current = this->stateManager->peek();
            if (current != nullptr) {
                if (current->getID() == STATE_MENU) {
                    MenuState* menu = (MenuState*)current;
                    int mode = menu->getSelectedMode();
                    if (mode == 99) {
                        this->running = false;
                        this->window.close();
                        return;
                    } else if (mode >= 0 && mode <= 2) {
                        // WHY: Transition from Menu to Play state
                        this->gameMode = mode;
                        this->stateManager->pop();
                        PlayState* play = new PlayState(this->gameMode);
                        this->stateManager->push(play);
                    }
                }
            }
        }
    }
}

void Game::update(float dt) {
    if (this->stateManager != nullptr) {
        this->stateManager->update(dt);
    }
}

void Game::render() {
    this->window.clear(Color::Black);
    if (this->stateManager != nullptr) {
        this->stateManager->render(this->window);
    }
    this->window.display();
}

void Game::cleanup() {
    if (this->stateManager != nullptr) {
        delete this->stateManager;
        this->stateManager = nullptr;
    }
    if (this->audManager != nullptr) {
        delete this->audManager;
        this->audManager = nullptr;
    }
    if (this->texManager != nullptr) {
        delete this->texManager;
        this->texManager = nullptr;
    }
}