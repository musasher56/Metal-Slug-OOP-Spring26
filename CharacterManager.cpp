#include "CharacterManager.h"
#include "PlayerSoldier.h"

CharacterManager::CharacterManager(TextureManager* texMgr, AudioManager* audMgr)
    : currentCharacter(0), kills(0), rings(0), fusionCompanion(nullptr),
      fusionAvailable(false), texManager(texMgr), audManager(audMgr) {
    
    for (int i = 0; i < 4; i++) {
        this->characters[i] = nullptr;
    }
    
    
    this->characters[0] = new Marco(texMgr, audMgr);
}

CharacterManager::~CharacterManager() {
    
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            delete this->characters[i];
            this->characters[i] = nullptr;
        }
    }
    
    
    this->fusionCompanion = nullptr;
}

void CharacterManager::update(float dt, Level* lvl) {
    
    PlayerSoldier* current = this->characters[this->currentCharacter];
    if (current == nullptr) return;

    
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        current->setDirectionAndVelocity(DIR_LEFT);
    }
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        current->setDirectionAndVelocity(DIR_RIGHT);
    }
    if (!Keyboard::isKeyPressed(Keyboard::Left) && !Keyboard::isKeyPressed(Keyboard::Right)) {
        current->decelerate();
    }

    
    current->update(0.0f, lvl);
}

void CharacterManager::draw(RenderWindow& window, float scrollX, float scrollY) {
    
    if (this->characters[this->currentCharacter] != nullptr) {
        this->characters[this->currentCharacter]->draw(window, scrollX, scrollY);
    }
}

void CharacterManager::handleInput(Event& event) {
    
    PlayerSoldier* current = this->characters[this->currentCharacter];
    if (current == nullptr) return;

    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Space) {
            current->handleJump();  
            
        }
    }
}

void CharacterManager::switchCharacter() {
    
    int startIdx = this->currentCharacter;
    do {
        this->currentCharacter = (this->currentCharacter + 1) % 4;
        if (this->characters[this->currentCharacter] != nullptr) {
            return;
        }
    } while (this->currentCharacter != startIdx);
}

void CharacterManager::takeDamage(int amount) {
    
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
    
}

void CharacterManager::loadData(float scroll) {
    
}
