#include "Game.h"
#include <cstdio>

Game::Game()
    : window(VideoMode(SCREEN_W, SCREEN_H), "Metal Slug - OOP SP26")
    , stateManager(nullptr)
    , texManager(nullptr)
    , audManager(nullptr)
    , running(true)
{
    this->window.setFramerateLimit(FRAMERATE_LIMIT);
    this->initialize();
}

Game::~Game() { this->cleanup(); }

void Game::initialize() {
    this->texManager = new TextureManager();
    this->audManager = new AudioManager();
    this->stateManager = new GameStateManager();
    MenuState* menu = new MenuState(this->texManager, this->audManager);
    this->stateManager->push(menu);
}

void Game::run() {
    Clock clock;
    while (this->running && this->window.isOpen()) {
        float dt = clock.restart().asSeconds();
        this->handleEvents();

        if (!this->running || !this->window.isOpen()) break;

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

        if (this->stateManager != nullptr) {
            this->stateManager->handleEvent(ev);

            GameState* current = this->stateManager->peek();
            if (current == nullptr) continue;

            
            if (current->getShouldExit()) {
                this->running = false;
                this->window.close();
                return;
            }

            
            if (current->getShouldGoBack()) {
                this->stateManager->pop();
                
                if (this->stateManager->peek() == nullptr) {
                    MenuState* menu = new MenuState(this->texManager, this->audManager);
                    this->stateManager->push(menu);
                }
                continue;
            }

            
            GameState* next = current->createNextState();
            if (next != nullptr) {
                
                next->gameMode = current->gameMode;
                next->selectedLevel = current->selectedLevel;
                next->selectedChar = current->selectedChar;
                if (next->stateManager == nullptr)
                    next->setStateManager(this->stateManager);

                this->stateManager->pop();
                this->stateManager->push(next);
            }
        }
    }
}

void Game::update(float dt) {
    if (this->stateManager) this->stateManager->update(dt);
}

void Game::render() {
    this->window.clear(Color::Black);
    if (this->stateManager) this->stateManager->render(this->window);
    this->window.display();
}

void Game::cleanup() {
    if (this->stateManager) {
        delete this->stateManager;
        this->stateManager = nullptr; }
    if (this->audManager) {
        delete this->audManager; 
        this->audManager = nullptr; }
    if (this->texManager) {
        delete this->texManager;
        this->texManager = nullptr; }
}
