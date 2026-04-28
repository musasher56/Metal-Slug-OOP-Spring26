#include "MenuState.h"

MenuState::MenuState(TextureManager* texMgr, AudioManager* audMgr) 
    : selectedOption(0), gameMode(MODE_SURVIVAL), texManager(texMgr), audManager(audMgr) {
    this->id = STATE_MENU;
}

MenuState::~MenuState() {
    // WHY: MenuState doesn't own texManager/audManager, just references them
}

void MenuState::update(float dt) {
    // WHY: Menu update logic (animations, hover effects) can go here
}

void MenuState::render(RenderWindow& window) {
    // WHY: Render menu background and options
    // TODO: Implement actual menu rendering with sprites
    window.clear(Color::Black);
    // Placeholder: draw simple text for menu options
}

void MenuState::handleEvent(Event& event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Up) {
            this->selectedOption--;
            if (this->selectedOption < 0) this->selectedOption = 2;
        } else if (event.key.code == Keyboard::Down) {
            this->selectedOption++;
            if (this->selectedOption > 2) this->selectedOption = 0;
        } else if (event.key.code == Keyboard::Enter) {
            this->gameMode = this->selectedOption;
            // WHY: Signal state manager to change state (handled by Game class)
        } else if (event.key.code == Keyboard::Escape) {
            this->gameMode = 99;  // Special code for quit
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
