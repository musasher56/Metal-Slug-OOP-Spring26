#include "MenuState.h"
#include "MainMenu.h"
#include "Charselectstate.h"

MenuState::MenuState(TextureManager* texMgr, AudioManager* audMgr) {
    this->id = GSTATE_MENU;
    this->gameMode = -1;
    this->selectedLevel = -1;
    this->texManager = texMgr;
    this->audManager = audMgr;
    this->stateManager = nullptr;
    this->shouldGoBack = false;
    this->shouldExit = false;

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
            this->shouldExit = true;
        }
    }

    // Sync context from MainMenu into GameState fields so createNextState()
    // can read them without knowing about MainMenu internals.
    if (this->mainMenu != nullptr) {
        this->gameMode = this->mainMenu->getSelectedMode();
        this->selectedLevel = this->mainMenu->getSelectedLevel();
    }
}

void MenuState::onEnter() {
    // Start title screen music when this state becomes active.
    if (this->audManager != nullptr) {
        this->audManager->playMusicTrack(0);  // Track 0 = title_theme.ogg
    }
}

void MenuState::onExit() {
    // Stop title music when leaving the menu (entering char select or exit).
    if (this->audManager != nullptr) {
        this->audManager->stopMusic();
    }
}

GameState* MenuState::createNextState() {
    // Only transition when MainMenu reports both mode and level are chosen
    if (this->mainMenu != nullptr && this->mainMenu->isReady()) {
        return new CharSelectState(this->texManager, this->audManager);
    }
    return nullptr;
}
