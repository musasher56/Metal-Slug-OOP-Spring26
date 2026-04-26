#include "Game.h"
#include <cstdio>

Game::Game()
    : window(VideoMode(SCREEN_W, SCREEN_H), "Metal Slug - OOP SP26")
    , texManager(nullptr)
    , audManager(nullptr)
    , mainMenu(nullptr)
    , level(nullptr)
    , player(nullptr)
    , gameMode(MODE_SURVIVAL)
    , running(true)
    , inMenu(true)
    , movingLeft(false)
    , movingRight(false)
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
    this->mainMenu   = new MainMenu(this->texManager, this->audManager);
}

void Game::startGame() {
    this->level  = new Level();
    this->player = new Player();
    this->window.requestFocus();

    this->bgTex.loadFromFile("resources/Sprites/tempbg.jpeg");
    this->bgSprite.setTexture(this->bgTex);
    float sx = (float)SCREEN_W / (float)this->bgTex.getSize().x;
    float sy = (float)SCREEN_H / (float)this->bgTex.getSize().y;
    this->bgSprite.setScale(sx, sy);
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

        if (this->inMenu && this->mainMenu != nullptr) {
            int result = this->mainMenu->handleEvent(ev);
            if (result == 99) {
                this->running = false;
                this->window.close();
            } else if (result >= 0) {
                this->gameMode = result;
                this->inMenu   = false;
                delete this->mainMenu;
                this->mainMenu = nullptr;
                this->startGame();
            }
        } else {
            if (ev.type == Event::KeyPressed) {
                if (ev.key.code == Keyboard::Right)  this->movingRight = true;
                if (ev.key.code == Keyboard::Left)   this->movingLeft  = true;
                if (ev.key.code == Keyboard::Space && this->player != nullptr)
                    this->player->jump();
                if (ev.key.code == Keyboard::Escape) {
                    this->running = false;
                    this->window.close();
                }
            }
            if (ev.type == Event::KeyReleased) {
                if (ev.key.code == Keyboard::Right) this->movingRight = false;
                if (ev.key.code == Keyboard::Left)  this->movingLeft  = false;
            }
        }
    }

    if (!this->inMenu && this->player != nullptr) {
        if (this->movingRight)     this->player->moveRight();
        else if (this->movingLeft) this->player->moveLeft();
        else                       this->player->stop();
    }
}

void Game::update(float dt) {
    if (this->inMenu && this->mainMenu != nullptr) {
        this->mainMenu->update(dt);
        return;
    }
    if (this->player != nullptr && this->level != nullptr) {
        this->player->update(
            this->level->getLvl(),
            this->level->getHeight(),
            this->level->getWidth(),
            this->level->getCellSize()
        );
    }
}

void Game::render() {
    this->window.clear(Color::Black);
    if (this->inMenu && this->mainMenu != nullptr) {
        this->mainMenu->draw(this->window);
    } else {
        this->window.draw(this->bgSprite);
        if (this->level  != nullptr) this->level->Draw(this->window);
        if (this->player != nullptr) this->player->draw(this->window);
    }
    this->window.display();
}

void Game::cleanup() {
    if (this->mainMenu  != nullptr) { delete this->mainMenu;  this->mainMenu  = nullptr; }
    if (this->player    != nullptr) { delete this->player;    this->player    = nullptr; }
    if (this->level     != nullptr) { delete this->level;     this->level     = nullptr; }
    if (this->audManager!= nullptr) { delete this->audManager;this->audManager= nullptr; }
    if (this->texManager!= nullptr) { delete this->texManager;this->texManager= nullptr; }
}