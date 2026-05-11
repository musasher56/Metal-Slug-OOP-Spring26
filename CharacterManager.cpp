#include "CharacterManager.h"
#include "PlayerSoldier.h"
#include "ProjectileManager.h"

CharacterManager::CharacterManager(TextureManager* texMgr, AudioManager* audMgr, int startChar)
    : currentCharacter(0), kills(0), rings(0), fusionCompanion(nullptr),
    fusionAvailable(false), texManager(texMgr), audManager(audMgr),
    pm(nullptr)
{
    for (int i = 0; i < 4; i++) {
        this->characters[i] = nullptr;
    }

    this->characters[0] = new Marco(texMgr, audMgr);
    this->characters[1] = new Tarma(texMgr, audMgr);
    this->characters[2] = new Eri(texMgr, audMgr);
    this->characters[3] = new Fio(texMgr, audMgr);

    
    
    if (startChar >= 0 && startChar < 4 && this->characters[startChar] != nullptr) {
        this->currentCharacter = startChar;
    }
    else {
        this->currentCharacter = 0;  
    }
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

void CharacterManager::setProjectileManager(ProjectileManager* manager) {
    
    this->pm = manager;

    
    
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            this->characters[i]->setProjectileManager(manager);
        }
    }
}

void CharacterManager::initAllPositions(sf::Vector2f startPos) {
    
    
    
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            this->characters[i]->position = startPos;
            this->characters[i]->updateBoundingBox();
        }
    }
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
            
            if (!current->getInWater()) {
                current->handleJump();
            }
        }
    }
}

void CharacterManager::switchCharacter() {
    
    
    PlayerSoldier* outgoing = this->characters[this->currentCharacter];

    int startIdx = this->currentCharacter;
    do {
        this->currentCharacter = (this->currentCharacter + 1) % 4;
        if (this->characters[this->currentCharacter] != nullptr) {

            PlayerSoldier* incoming = this->characters[this->currentCharacter];

            
            
            
            if (outgoing != nullptr) {
                incoming->copyPhysicsFrom(outgoing);
            }

            
            
            
            
            
            
            
            
            
            
            
            if (outgoing != nullptr) {
                int outH = outgoing->getBoundingBox().height;
                incoming->updateBoundingBox();           
                int inH = incoming->getBoundingBox().height;
                int delta = outH - inH;                  
                if (delta != 0) {
                    incoming->position.y += static_cast<float>(delta);
                }
            }

            
            incoming->updateBoundingBox();

            
            if (this->pm != nullptr) {
                incoming->setProjectileManager(this->pm);
            }

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