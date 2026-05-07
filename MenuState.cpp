#include "MenuState.h"

MenuState::MenuState(TextureManager* texMgr, AudioManager* audMgr) 
    : mainMenu(nullptr), texManager(texMgr), audManager(audMgr) {
    this->id = GSTATE_MENU;
    this->gameMode = -1; 
    
    this->mainMenu = new MainMenu(this->texManager, this->audManager);
}

MenuState::~MenuState() {
    
    if (this->mainMenu != nullptr) {
        delete this->mainMenu;
        this->mainMenu = nullptr;
    }
    
}

void MenuState::update(float dt) {
    
    if (this->mainMenu != nullptr) {
        this->mainMenu->update(dt);
    }
}

void MenuState::render(RenderWindow& window) {
    
    if (this->mainMenu != nullptr) {
        this->mainMenu->draw(window);
    }
}

void MenuState::handleEvent(Event& event) {
    
    if (this->mainMenu != nullptr) {
        int result = this->mainMenu->handleEvent(event);
        if (result == 99) {
            
            this->gameMode = 99;
        } else if (result >= 0 && result <= 2) {
            
            this->gameMode = result;
        }
    }
}

void MenuState::onEnter() {
    
}

void MenuState::onExit() {
    
}

int MenuState::getSelectedMode() const {
    return this->gameMode;
}
