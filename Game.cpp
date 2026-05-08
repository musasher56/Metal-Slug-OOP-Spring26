#include "Game.h"
#include <cstdio>

Game::Game()
    : window(VideoMode(SCREEN_W, SCREEN_H), "Metal Slug - OOP SP26")
    , stateManager(nullptr)
    , texManager(nullptr)
    , audManager(nullptr)
    , gameMode(MODE_SURVIVAL)
    , selectedChar(0)
    , running(true)
{
    this->window.setFramerateLimit(FRAMERATE_LIMIT);
    this->initialize();
}

Game::~Game() { this->cleanup(); }

void Game::initialize() {
    this->texManager   = new TextureManager();
    this->audManager   = new AudioManager();
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
        if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape) {
            this->running = false;
            this->window.close();
            return;
        }
        if (this->stateManager != nullptr) {
            this->stateManager->handleEvent(ev);
            GameState* current = this->stateManager->peek();

            // ── MenuState: mode selected → go to character select ────────────
            if (current != nullptr && current->getID() == GSTATE_MENU) {
                MenuState* menu = (MenuState*)current;
                int mode = menu->getSelectedMode();
                if (mode == 99) {
                    this->running = false;
                    this->window.close();
                    return;
                } else if (mode >= 0 && mode <= 2) {
                    this->gameMode = mode;
                    this->stateManager->pop();
                    // Push character select screen — player chooses their soldier
                    CharSelectState* charSel = new CharSelectState(
                        this->texManager, this->audManager);
                    this->stateManager->push(charSel);
                }
            }

            // ── CharSelectState: character chosen → launch PlayState ─────────
            else if (current != nullptr && current->getID() == GSTATE_CHAR_SELECT) {
                CharSelectState* charSel = (CharSelectState*)current;
                int chosen = charSel->getSelectedChar();
                if (chosen >= 0 && chosen <= 3) {
                    this->selectedChar = chosen;
                    this->stateManager->pop();
                    PlayState* play = new PlayState(this->gameMode,
                                                    this->selectedChar,
                                                    this->texManager,
                                                    this->audManager);
                    this->stateManager->push(play);
                }
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
    if (this->stateManager) { delete this->stateManager; this->stateManager = nullptr; }
    if (this->audManager)   { delete this->audManager;   this->audManager   = nullptr; }
    if (this->texManager)   { delete this->texManager;   this->texManager   = nullptr; }
}