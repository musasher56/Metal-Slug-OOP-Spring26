#include "CharacterManager.h"
#include "PlayerSoldier.h"

CharacterManager::CharacterManager(TextureManager* texMgr, AudioManager* audMgr)
    : currentCharacter(0), kills(0), rings(0), fusionCompanion(nullptr),
      fusionAvailable(false), texManager(texMgr), audManager(audMgr) {
    // WHY: Initialize all 4 character slots to nullptr
    for (int i = 0; i < 4; i++) {
        this->characters[i] = nullptr;
    }
    
    // WHY: Create Marco as the default character (index 0)
    this->characters[0] = new Marco(texMgr, audMgr);
}

CharacterManager::~CharacterManager() {
    // WHY: Delete all owned PlayerSoldiers
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            delete this->characters[i];
            this->characters[i] = nullptr;
        }
    }
    // WHY: fusionCompanion is a stub - actual deletion will happen when FusionCompanion is fully implemented
    // For now, just set to nullptr to avoid incomplete type issues
    this->fusionCompanion = nullptr;
}

void CharacterManager::update(float dt, Level* lvl) {
    // ROOT CAUSE 3 & 4 FIX: Always update character, poll input here (frame-based)
    PlayerSoldier* current = this->characters[this->currentCharacter];
    if (current == nullptr) return;

    // Poll real-time input here (movement is frame-based, not event-based)
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        current->setDirectionAndVelocity(DIR_LEFT);
    }
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        current->setDirectionAndVelocity(DIR_RIGHT);
    }
    if (!Keyboard::isKeyPressed(Keyboard::Left) && !Keyboard::isKeyPressed(Keyboard::Right)) {
        current->decelerate();
    }

    // ROOT CAUSE 3 FIX: lvl can be nullptr — Soldier::handleCollision null-checks it
    current->update(0.0f, lvl);
}

void CharacterManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    // WHY: Draw current active character with both scroll offsets for vertical scrolling
    if (this->characters[this->currentCharacter] != nullptr) {
        this->characters[this->currentCharacter]->draw(window, scrollX, scrollY);
    }
}

void CharacterManager::handleInput(Event& event) {
    // ROOT CAUSE 4 FIX: Handle jump event (event-based, not polled)
    PlayerSoldier* current = this->characters[this->currentCharacter];
    if (current == nullptr) return;

    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Space) {
            current->handleJump();  // handleJump is protected, but CharacterManager needs access
            // Note: May need to make handleJump public or add a public jump() wrapper
        }
    }
}

void CharacterManager::switchCharacter() {
    // WHY: Cycle to next alive character
    int startIdx = this->currentCharacter;
    do {
        this->currentCharacter = (this->currentCharacter + 1) % 4;
        if (this->characters[this->currentCharacter] != nullptr) {
            return;
        }
    } while (this->currentCharacter != startIdx);
}

void CharacterManager::takeDamage(int amount) {
    // WHY: Apply damage to current character
    if (this->characters[this->currentCharacter] != nullptr) {
        this->characters[this->currentCharacter]->takeDamage(amount);
    }
}

PlayerSoldier* CharacterManager::getCurrentCharacter() {
    return this->characters[this->currentCharacter];
}

int CharacterManager::getCurrentCharacterIdx() {
    return this->currentCharacter;
}

int CharacterManager::getHealthPoints() {
    if (this->characters[this->currentCharacter] != nullptr) {
        return this->characters[this->currentCharacter]->getCurrentHP();
    }
    return 0;
}

int CharacterManager::getRings() {
    return this->rings;
}

int CharacterManager::getKills() {
    return this->kills;
}

void CharacterManager::incKills() {
    this->kills++;
}

void CharacterManager::spawnFusion() {
    // TODO: Implement FusionCompanion spawning
}

bool CharacterManager::isFusionActive() {
    return this->fusionCompanion != nullptr;
}

bool CharacterManager::anyCharacterAlive() {
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr && this->characters[i]->isAlive()) {
            return true;
        }
    }
    return false;
}

void CharacterManager::saveData(float scroll) {
    // TODO: Implement save
}

void CharacterManager::loadData(float scroll) {
    // TODO: Implement load
}
