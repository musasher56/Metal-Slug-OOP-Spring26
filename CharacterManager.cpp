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

    // Clamp startChar to a valid slot so a bad value from CharSelectState
    // (e.g. -1 or 4) never leaves currentCharacter pointing at nullptr.
    if (startChar >= 0 && startChar < 4 && this->characters[startChar] != nullptr) {
        this->currentCharacter = startChar;
    }
    else {
        this->currentCharacter = 0;  // fallback to Marco
    }
}

CharacterManager::~CharacterManager() {
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            delete this->characters[i];
            this->characters[i] = nullptr;
        }
    }
    // pm is aggregated (owned by PlayState), never deleted here.
    this->fusionCompanion = nullptr;
}

void CharacterManager::setProjectileManager(ProjectileManager* manager) {
    // Store so future switches can forward the pointer to incoming characters.
    this->pm = manager;

    // Distribute immediately to every existing slot so no character ever
    // tries to fire with a null pm, regardless of which one is active first.
    for (int i = 0; i < 4; i++) {
        if (this->characters[i] != nullptr) {
            this->characters[i]->setProjectileManager(manager);
        }
    }
}

void CharacterManager::initAllPositions(sf::Vector2f startPos) {
    // Sets every character slot to the same world position so they all
    // start standing on the surface rather than at the constructor-default
    // (200, 300) which may be mid-air or inside terrain depending on the level.
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
            current->handleJump();
        }
        if (event.key.code == Keyboard::Down) {
            current->handleSwimDown();
        }
    }
}

void CharacterManager::switchCharacter() {
    // Capture outgoing character before advancing the index.
    // We need its physics state to hand to the incoming character.
    PlayerSoldier* outgoing = this->characters[this->currentCharacter];

    int startIdx = this->currentCharacter;
    do {
        this->currentCharacter = (this->currentCharacter + 1) % 4;
        if (this->characters[this->currentCharacter] != nullptr) {

            PlayerSoldier* incoming = this->characters[this->currentCharacter];

            // --- Physics handoff ---
            // copyPhysicsFrom() writes position, velocityX/Y, onGround, and
            // direction into the incoming soldier. All five are protected on
            // Soldier, so this must be a member function — not a direct field
            // copy from CharacterManager.
            //
            // Without this the incoming character spawns at (200, 300),
            // which is almost certainly inside terrain at any non-trivial
            // level scroll, causing the "fall through floor" glitch.
            if (outgoing != nullptr) {
                incoming->copyPhysicsFrom(outgoing);
            }

            // Rebuild the bounding box for the incoming character's sprite
            // after the position change. The box stores an offset from
            // position, so it must be recalculated now or the first collision
            // frame will use stale geometry.
            incoming->updateBoundingBox();

            // Ensure pm is valid for the incoming character. Even though
            // setProjectileManager() distributes to all slots at init,
            // this guard makes the switch self-contained: a character
            // constructed after init (future feature) would still work.
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