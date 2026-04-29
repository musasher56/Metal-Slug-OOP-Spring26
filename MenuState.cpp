#include "MenuState.h"

MenuState::MenuState(TextureManager* texMgr, AudioManager* audMgr) 
    : mainMenu(nullptr), texManager(texMgr), audManager(audMgr) {
    this->id = STATE_MENU;
    this->gameMode = -1; // prevent auto-transition before a selection
    // WHY: Create MainMenu instance which handles video frames and menu rendering
    this->mainMenu = new MainMenu(this->texManager, this->audManager);
}

MenuState::~MenuState() {
    // WHY: MenuState owns the MainMenu instance, must delete it
    if (this->mainMenu != nullptr) {
        delete this->mainMenu;
        this->mainMenu = nullptr;
    }
    // WHY: MenuState doesn't own texManager/audManager, just references them
}

void MenuState::update(float dt) {
    // WHY: Delegate update to MainMenu for video frame animations
    if (this->mainMenu != nullptr) {
        this->mainMenu->update(dt);
    }
}

void MenuState::render(RenderWindow& window) {
    // WHY: Delegate rendering to MainMenu which handles video frames and menu UI
    if (this->mainMenu != nullptr) {
        this->mainMenu->draw(window);
    }
}

void MenuState::handleEvent(Event& event) {
    // WHY: Delegate event handling to MainMenu, check for mode selection
    if (this->mainMenu != nullptr) {
        int result = this->mainMenu->handleEvent(event);
        if (result == 99) {
            // WHY: Signal exit request
            this->gameMode = 99;
        } else if (result >= 0 && result <= 2) {
            // WHY: Signal mode selection (handled by Game class)
            this->gameMode = result;
        }
    }
}

void MenuState::onEnter() {
    // WHY: Initialize menu state when entering
}

void MenuState::onExit() {
    // WHY: Cleanup menu state when exiting
}

int MenuState::getSelectedMode() const {
    return this->gameMode;
}
