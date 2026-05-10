#include "Soldier.h"
#include "Level.h"
#include <SFML/Window/Keyboard.hpp>

Soldier::Soldier(TextureManager* texMgr, AudioManager* audMgr)
    : DamagableEntity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , maxVelocity(5.f)
    , baseMaxVelocity(5.f)
    , direction(DIR_RIGHT)
    , onGround(false)
    , lives(3)
    , currentHP(3)
    , saturation(100)
    , meleeDamage(1.f)
    , meleeCooldown(0.5f)
    , transformState(nullptr), isInvincible(false)
    , inWater(false)
    , physW(34)   // matches original hardcoded value; subclasses override
    , physH(40)   // matches original hardcoded value; subclasses override
{
}

Soldier::~Soldier() {
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
    }
    this->transformState = nullptr;
}

void Soldier::update(float scroll, Level* lvl) {
    if (this->isInvincible && this->invincibilityClock.getElapsedTime().asSeconds() >= 2.0f) {
        this->isInvincible = false;

    }
    this->handleStateTimers();
    if (this->inWater) {
        this->applyWaterPhysics();
        // ── Swimming controls (checked every frame) ──
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            this->velocityY -= 0.8f;
            if (this->velocityY < -4.f) this->velocityY = -4.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            this->velocityY += 0.8f;
            if (this->velocityY > 4.f) this->velocityY = 4.f;
        }
    }
    else {
        this->applyGravity();
    }
    this->applyMovement(scroll);
    this->handleCollision(lvl);
}

void Soldier::draw(RenderWindow& window, float scrollX, float scrollY) {
    if (this->velocityX != 0.f || !this->onGround) {
        this->animation.update();
    }
    else {
        this->animation.currentFrame = 5;
        this->animation.clock.restart();
    }
    this->animation.applyToSprite(this->sprite);
    this->sprite.setPosition(this->position.x - scrollX, this->position.y - scrollY);
    window.draw(this->sprite);
}

void Soldier::takeDamage(int amount) {





    if (amount < 0) return;

    if (this->transformState != nullptr) {

    }

    this->currentHP -= amount;
    if (this->currentHP < 0) this->currentHP = 0;

    if (this->currentHP == 0) {
        this->onDeath();
    }
    else if (this->currentHP == 1) {

    }
    else if (this->currentHP == 2) {

    }



    this->health = this->currentHP;
}

void Soldier::meleeAttack() {
    if (this->meleeTimer.getElapsedTime().asSeconds() >= this->meleeCooldown) {
        this->meleeTimer.restart();
    }
}

int Soldier::getState() const {
    return this->currentHP;
}

int Soldier::getLives() const {
    return this->lives;
}

int Soldier::getCurrentHP() const {
    return this->currentHP;
}

void Soldier::respawn() {
    this->currentHP = 3;
    this->health = 3;
    this->maxHealth = 3;
    this->position.x = 100.f;
    this->position.y = 300.f;
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->direction = DIR_RIGHT;
    this->onGround = false;
    this->status = true;
    // Grant 2 seconds of invincibility after respawn to prevent death loops
    this->isInvincible = true;
    this->invincibilityClock.restart();
}

void Soldier::setTransformationState(TransformationState* newState) {
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
        this->transformState = nullptr;
    }

    this->transformState = newState;

    if (this->transformState != nullptr) {
        this->transformState->applyEffects(this);
    }
}

TransformationState* Soldier::getTransformationState() const {
    return this->transformState;
}

void Soldier::handleJump() {
    if (this->onGround) {
        this->velocityY = -20.f;
        this->onGround = false;
    }
}

void Soldier::setInWater(bool val) {
    this->inWater = val;
}

bool Soldier::getInWater() const {
    return this->inWater;
}

void Soldier::applyGravity() {
    if (!this->onGround) {
        this->velocityY += 0.8f;
        if (this->velocityY > 20.f) {
            this->velocityY = 20.f;
        }
    }
}

void Soldier::applyWaterPhysics() {
    // Neutral buoyancy: no gravity in water
    // Moderate drag so player can actually move but still slows down
    this->velocityX *= 0.96f;
    this->velocityY *= 0.96f;

    // Snap tiny velocities to zero so player actually stops
    if (this->velocityX > -0.2f && this->velocityX < 0.2f) this->velocityX = 0.f;
    if (this->velocityY > -0.2f && this->velocityY < 0.2f) this->velocityY = 0.f;
}

void Soldier::handleCollision(Level* lvl) {
    if (lvl == nullptr) return;

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);

    // ── CRITICAL: use physW/physH, NOT hardcoded values ──────────────────
    // physW and physH are set per-character in each subclass constructor to
    // match that character's actual sprite frame size.  Multiplying by the
    // sprite's current scale gives the exact on-screen collision extent.
    //
    // Old code used: 34.f * scaleX, 40.f * scaleY
    // That was fine for Marco (physW=34, physH=40, scale=3.5) but broke
    // Fio (physH=18 at scale 8.0 → old formula gave 40*8=320px box height
    // vs the 18*8=144px visual sprite → collision pushed Fio 176px up → float).
    float colW = static_cast<float>(this->physW) * scaleX;
    float colH = static_cast<float>(this->physH) * scaleY;

    float playerLeft = this->position.x;
    float playerRight = this->position.x + colW;
    float playerTop = this->position.y;
    float playerBottom = this->position.y + colH;

    int cellSize = lvl->getCellSize();
    int worldOffX = lvl->getWorldOffX();   // campaign: -30; survival: 0
    this->onGround = false;

    // ── WORLD-TO-GRID CONVERSION ──────────────────────────────────────────
    // Grid column c corresponds to world X = (worldOffX + c) * cellSize.
    // So to convert world X to grid column: col = (worldX / cellSize) - worldOffX.
    // In survival mode worldOffX = 0, so this reduces to the old formula.
    int startCol = static_cast<int>(playerLeft) / cellSize - worldOffX - 1;
    int endCol = static_cast<int>(playerRight) / cellSize - worldOffX + 1;
    int startRow = static_cast<int>(playerTop) / cellSize - 1;
    int endRow = static_cast<int>(playerBottom) / cellSize + 1;

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) continue;

            // Grid-to-world: world X = (worldOffX + col) * cellSize
            float blockLeft = static_cast<float>((worldOffX + col) * cellSize);
            float blockRight = blockLeft + static_cast<float>(cellSize);
            float blockTop = static_cast<float>(row * cellSize);
            float blockBottom = blockTop + static_cast<float>(cellSize);

            if (playerRight > blockLeft && playerLeft < blockRight &&
                playerBottom > blockTop && playerTop < blockBottom) {

                float overlapLeft = playerRight - blockLeft;
                float overlapRight = blockRight - playerLeft;
                float overlapTop = playerBottom - blockTop;
                float overlapBottom = blockBottom - playerTop;

                float minOverlap = overlapLeft;
                int   resolveDir = 1;

                if (overlapRight < minOverlap) { minOverlap = overlapRight;  resolveDir = 2; }
                if (overlapTop < minOverlap) { minOverlap = overlapTop;    resolveDir = 3; }
                if (overlapBottom < minOverlap) { minOverlap = overlapBottom; resolveDir = 4; }

                if (resolveDir == 1) {
                    this->position.x -= minOverlap;
                    this->velocityX = 0.f;
                }
                else if (resolveDir == 2) {
                    this->position.x += minOverlap;
                    this->velocityX = 0.f;
                }
                else if (resolveDir == 3) {
                    this->position.y -= minOverlap;
                    this->velocityY = 0.f;
                    this->onGround = true;
                }
                else if (resolveDir == 4) {
                    this->position.y += minOverlap;
                    this->velocityY = 0.f;
                }

                // Recompute after resolution so the next block uses the new position
                playerLeft = this->position.x;
                playerRight = this->position.x + colW;
                playerTop = this->position.y;
                playerBottom = this->position.y + colH;
            }
        }
    }

    // Ground probe: check one pixel below the collision box for a solid tile.
    // Keeps onGround = true even when standing perfectly flush (no overlap).
    if (!this->onGround) {
        float probeY = playerBottom + 1.0f;
        int probeRow = static_cast<int>(probeY) / cellSize;
        int probeStartCol = static_cast<int>(playerLeft + 2) / cellSize - worldOffX;
        int probeEndCol = static_cast<int>(playerRight - 2) / cellSize - worldOffX;
        for (int col = probeStartCol; col <= probeEndCol; ++col) {
            if (lvl->isSolid(probeRow, col)) {
                this->onGround = true;
                break;
            }
        }
    }

    // Left-edge world clamp
    // The leftmost valid world X is worldOffX * cellSize (grid column 0).
    float minWorldX = static_cast<float>(worldOffX) * static_cast<float>(cellSize);
    if (this->position.x < minWorldX) {
        this->position.x = minWorldX;
        this->velocityX = 0.f;
    }

    // Right-edge world clamp (also uses physW so every character is clamped correctly)
    float maxPlayerX = static_cast<float>(worldOffX + lvl->getWidth()) * static_cast<float>(lvl->getCellSize()) - colW;
    if (this->position.x > maxPlayerX) {
        this->position.x = maxPlayerX;
        this->velocityX = 0.f;
    }
}

void Soldier::applyMovement(float& scroll) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;

    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = this->sprite.getScale().y;
    if (this->direction == DIR_LEFT) {
        this->sprite.setScale(-scaleX, scaleY);
    }
    else {
        this->sprite.setScale(scaleX, scaleY);
    }
}

void Soldier::handleStateTimers() {
    if (this->transformState != nullptr) {
        float dt = 0.016f;
        this->transformState->update(this, dt);

        if (this->transformState->isExpired()) {
            this->transformState->onExpiry(this);
        }
    }
}

void Soldier::setDirectionAndVelocity(int dir) {
    this->direction = dir;
    float accel = this->inWater ? 0.3f : 0.5f;
    float maxV = this->inWater ? 3.f : this->maxVelocity;
    if (dir == DIR_LEFT) {
        this->velocityX -= accel;
        if (this->velocityX < -maxV) this->velocityX = -maxV;
    }
    else {
        this->velocityX += accel;
        if (this->velocityX > maxV) this->velocityX = maxV;
    }
}

void Soldier::decelerate() {
    float drag = this->inWater ? 0.25f : 0.5f;
    if (this->onGround || this->inWater) {
        if (this->velocityX > 0.f) {
            this->velocityX -= drag;
            if (this->velocityX < 0.f) this->velocityX = 0.f;
        }
        else if (this->velocityX < 0.f) {
            this->velocityX += drag;
            if (this->velocityX > 0.f) this->velocityX = 0.f;
        }
    }
}

void Soldier::copyPhysicsFrom(Soldier* other) {
    // Transfers the complete physics snapshot from 'other' to this soldier.
    // Called by CharacterManager::switchCharacter() so the incoming character
    // appears exactly where the outgoing one was — same tile, same momentum,
    // same facing direction — rather than teleporting to its constructor default.
    //
    // position is public on Entity so we assign it directly.
    // velocityX/Y, onGround, direction are protected on Soldier, so this
    // method lives here to legally access both sides without breaking
    // encapsulation or requiring friend declarations.
    if (other == nullptr) return;

    this->position = other->position;
    this->velocityX = other->velocityX;
    this->velocityY = other->velocityY;
    this->onGround = other->onGround;
    this->direction = other->direction;
}

void Soldier::resolveBlockCollisions(DamagableEntity** blocks, int count) {
    if (blocks == nullptr || count == 0) return;

    IntRect pb = this->getBoundingBox();
    float playerLeft = static_cast<float>(pb.left);
    float playerRight = static_cast<float>(pb.left + pb.width);
    float playerTop = static_cast<float>(pb.top);
    float playerBottom = static_cast<float>(pb.top + pb.height);

    for (int i = 0; i < count; i++) {
        if (blocks[i] == nullptr) continue;
        if (!blocks[i]->isAlive() || !blocks[i]->getStatus()) continue;

        IntRect b = blocks[i]->getBoundingBox();

        float blockLeft = static_cast<float>(b.left);
        float blockRight = static_cast<float>(b.left + b.width);
        float blockTop = static_cast<float>(b.top);
        float blockBottom = static_cast<float>(b.top + b.height);

        if (playerRight <= blockLeft || playerLeft >= blockRight ||
            playerBottom <= blockTop || playerTop >= blockBottom) continue;

        float overlapLeft = playerRight - blockLeft;
        float overlapRight = blockRight - playerLeft;
        float overlapTop = playerBottom - blockTop;
        float overlapBottom = blockBottom - playerTop;

        bool movingDown = (this->velocityY > 0.5f);
        bool movingUp = (this->velocityY < -0.5f);
        bool movingHoriz = (this->velocityX > 0.5f || this->velocityX < -0.5f);

        float minOverlap = overlapLeft;
        int   resolveDir = 1;

        if (overlapRight < minOverlap) { minOverlap = overlapRight;  resolveDir = 2; }
        if (overlapTop < minOverlap) { minOverlap = overlapTop;    resolveDir = 3; }
        if (overlapBottom < minOverlap) { minOverlap = overlapBottom; resolveDir = 4; }

        if (movingHoriz && !movingDown) {
            float sideOverlap = overlapLeft;
            int   sideDir = 1;
            if (overlapRight < sideOverlap) { sideOverlap = overlapRight; sideDir = 2; }
            if (sideOverlap < overlapTop * 2.f) {
                minOverlap = sideOverlap;
                resolveDir = sideDir;
            }
        }

        if (resolveDir == 1) {
            this->position.x -= minOverlap;
            this->velocityX = 0.f;
        }
        else if (resolveDir == 2) {
            this->position.x += minOverlap;
            this->velocityX = 0.f;
        }
        else if (resolveDir == 3) {
            this->position.y -= minOverlap;
            this->velocityY = 0.f;
            this->onGround = true;
        }
        else if (resolveDir == 4) {
            this->position.y += minOverlap;
            if (this->velocityY < 0.f) this->velocityY = 0.f;
        }

        pb = this->getBoundingBox();
        playerLeft = static_cast<float>(pb.left);
        playerRight = static_cast<float>(pb.left + pb.width);
        playerTop = static_cast<float>(pb.top);
        playerBottom = static_cast<float>(pb.top + pb.height);
    }
}