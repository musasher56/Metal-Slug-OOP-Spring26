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
    if (this->fusionCompanion != nullptr) {
        delete this->fusionCompanion;
        this->fusionCompanion = nullptr;
    }
}

void CharacterManager::update(float dt, Level* lvl) {
    // WHY: Update current active character
    if (this->characters[this->currentCharacter] != nullptr && lvl != nullptr) {
        this->characters[this->currentCharacter]->update(0.0f, lvl);
    }
}

void CharacterManager::draw(RenderWindow& window) {
    // WHY: Draw current active character
    if (this->characters[this->currentCharacter] != nullptr) {
        this->characters[this->currentCharacter]->draw(window, 0.0f);
    }
}

void CharacterManager::handleInput(Event& event) {
    // WHY: Delegate input to current character's handleInput
    if (this->characters[this->currentCharacter] != nullptr) {
        // Character-specific input handled inside PlayerSoldier::handleInput
        // handleInput delegated to PlayerSoldier::update();
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
