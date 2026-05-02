#include "Soldier.h"
#include "Level.h"

Soldier::Soldier(TextureManager* texMgr, AudioManager* audMgr)
    : DamagableEntity(texMgr, audMgr)
    , velocityX(0.f)
    , velocityY(0.f)
    , maxVelocity(5.f)
    , baseMaxVelocity(5.f)
    , direction(DIR_RIGHT)
    , onGround(false)
    , lives(3)
    , currentHP(3)  // HEALTHY
    , saturation(100)
    , meleeDamage(1.f)
    , meleeCooldown(0.5f)
    , transformState(nullptr)
{
}

Soldier::~Soldier() {
    // WHY: FIX #4 - Soldier does not own NormalState (singleton), but owns other states
    // NormalState has type TRANSFORM_NONE, so we only delete if type != TRANSFORM_NONE
    if (this->transformState != nullptr && this->transformState->getType() != TRANSFORM_NONE) {
        delete this->transformState;
    }
    this->transformState = nullptr;
}

void Soldier::update(float scroll, Level* lvl) {
    this->handleStateTimers();
    this->applyGravity();
    this->applyMovement(scroll);
    this->handleCollision(lvl);
}

void Soldier::draw(RenderWindow& window, float scroll) {
    // Only animate when moving or in the air; reset to frame 5 when idle on ground
    if (this->velocityX != 0.f || !this->onGround) {
        this->animation.update();                          // advance frame
    }
    else {
        this->animation.currentFrame = 5;                  // snap back to idle frame (index 5)
        this->animation.clock.restart();                   // restart clock to prevent jump
    }
    this->animation.applyToSprite(this->sprite);       // apply to sprite
    this->sprite.setPosition(this->position.x - scroll, this->position.y);
    window.draw(this->sprite);
}

void Soldier::takeDamage(int amount) {
    if (amount < 0) return;

    // Check if transformation provides immunity or reduction
    if (this->transformState != nullptr) {
        // Undead state could have different damage handling
    }

    this->currentHP -= amount;
    if (this->currentHP < 0) this->currentHP = 0;

    // Update health state enum
    if (this->currentHP == 0) {
        this->onDeath();
    }
    else if (this->currentHP == 1) {
        // CRITICAL state
    }
    else if (this->currentHP == 2) {
        // INJURED state
    }

    DamagableEntity::takeDamage(amount);
}

void Soldier::meleeAttack() {
    if (this->meleeTimer.getElapsedTime().asSeconds() >= this->meleeCooldown) {
        // Perform melee attack logic here
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
    this->health = this->maxHealth;
    this->position.x = 100.f;
    this->position.y = 300.f;
    this->velocityX = 0.f;
    this->velocityY = 0.f;
    this->onGround = false;
}

void Soldier::setTransformationState(TransformationState* newState) {
    // WHY: FIX #4 - Delete previous state if it is NOT NormalState (singleton)
    // NormalState is a static singleton and must never be deleted
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
        this->velocityY = -15.f;  // Jump strength (increased to clear blocks)
        this->onGround = false;
    }
}

void Soldier::applyGravity() {
    if (!this->onGround) {
        this->velocityY += 0.8f;  // Gravity acceleration
        if (this->velocityY > 20.f) {
            this->velocityY = 20.f;  // Terminal velocity
        }
    }
}

void Soldier::handleCollision(Level* lvl) {
    // ROOT CAUSE 3 FIX: Null-guard handleCollision against null level
    if (lvl == nullptr) return;  // no collision without a level — safe to skip

    // Get player bounding box - use abs() on scale to handle flipped sprites
    float scaleX = std::abs(this->sprite.getScale().x);
    float scaleY = std::abs(this->sprite.getScale().y);
    float playerLeft = this->position.x;
    float playerRight = this->position.x + 32.f * scaleX;
    float playerTop = this->position.y;
    float playerBottom = this->position.y + 40.f * scaleY;

    int cellSize = lvl->getCellSize();
    this->onGround = false;

    // Check cells around the player
    int startCol = static_cast<int>(playerLeft) / cellSize - 1;
    int endCol = static_cast<int>(playerRight) / cellSize + 1;
    int startRow = static_cast<int>(playerTop) / cellSize - 1;
    int endRow = static_cast<int>(playerBottom) / cellSize + 1;

    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (!lvl->isSolid(row, col)) continue;

            // Solid block bounds
            float blockLeft = static_cast<float>(col * cellSize);
            float blockRight = blockLeft + static_cast<float>(cellSize);
            float blockTop = static_cast<float>(row * cellSize);
            float blockBottom = blockTop + static_cast<float>(cellSize);

            // Check for overlap
            if (playerRight > blockLeft && playerLeft < blockRight &&
                playerBottom > blockTop && playerTop < blockBottom) {

                // Collision detected - resolve based on velocity
                float overlapLeft = playerRight - blockLeft;
                float overlapRight = blockRight - playerLeft;
                float overlapTop = playerBottom - blockTop;
                float overlapBottom = blockBottom - playerTop;

                // Find minimum overlap
                float minOverlap = overlapLeft;
                int resolveDir = 0; // 1=left, 2=right, 3=top, 4=bottom

                if (overlapRight < minOverlap) {
                    minOverlap = overlapRight;
                    resolveDir = 2;
                }
                if (overlapTop < minOverlap) {
                    minOverlap = overlapTop;
                    resolveDir = 3;
                }
                if (overlapBottom < minOverlap) {
                    minOverlap = overlapBottom;
                    resolveDir = 4;
                }

                // Resolve collision
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

                // Update player bounds after resolution - use abs() on scale
                playerLeft = this->position.x;
                playerRight = this->position.x + 32.f * scaleX;
                playerTop = this->position.y;
                playerBottom = this->position.y + 40.f * scaleY;
            }
        }
    }

    // Ground probe: detect solid tiles 1px below feet when no overlap exists
    if (!this->onGround) {
        float probeY = playerBottom + 1.0f;  // 1px tolerance below feet
        int probeRow = static_cast<int>(probeY) / cellSize;
        int probeStartCol = static_cast<int>(playerLeft + 2) / cellSize;
        int probeEndCol = static_cast<int>(playerRight - 2) / cellSize;
        for (int col = probeStartCol; col <= probeEndCol; ++col) {
            if (lvl->isSolid(probeRow, col)) {
                this->onGround = true;
                break;
            }
        }
    }

    // FIX: Invisible walls at level boundaries
    if (this->position.x < 0.f) {
        this->position.x = 0.f;
        this->velocityX = 0.f;
    }
    float maxPlayerX = (float)(lvl->getWidth()) * (float)(lvl->getCellSize())
        - 32.f * scaleX;  // account for player width
    if (this->position.x > maxPlayerX) {
        this->position.x = maxPlayerX;
        this->velocityX = 0.f;
    }
}

void Soldier::applyMovement(float& scroll) {
    this->position.x += this->velocityX;
    this->position.y += this->velocityY;

    // Apply direction to sprite - preserve existing scale magnitude, only flip X sign
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
        float dt = 0.016f;  // Approximate frame time
        this->transformState->update(this, dt);

        if (this->transformState->isExpired()) {
            this->transformState->onExpiry(this);
        }
    }
}

// ROOT CAUSE 4 FIX: Helper methods for movement control from CharacterManager
void Soldier::setDirectionAndVelocity(int dir) {
    this->direction = dir;
    float accel = 0.5f;
    if (dir == DIR_LEFT) {
        this->velocityX -= accel;
        if (this->velocityX < -this->maxVelocity) this->velocityX = -this->maxVelocity;
    }
    else {
        this->velocityX += accel;
        if (this->velocityX > this->maxVelocity) this->velocityX = this->maxVelocity;
    }
}

void Soldier::decelerate() {
    if (this->onGround) {
        if (this->velocityX > 0.f) {
            this->velocityX -= 0.5f;
            if (this->velocityX < 0.f) this->velocityX = 0.f;
        }
        else if (this->velocityX < 0.f) {
            this->velocityX += 0.5f;
            if (this->velocityX > 0.f) this->velocityX = 0.f;
        }
    }
}

// ============================================================
// resolveBlockCollisions
// ============================================================
// Called from PlayState::update() after block list is built.
// Resolves the player against each live block's world-space bounding box
// using the same min-overlap logic as handleCollision().
// WHY separate from handleCollision()?
//   handleCollision() reads solid tiles from Level. Blocks are NOT marked
//   solid in the level grid (that would stop bullets — see Block.cpp notes).
//   So block collision must be handled here, against entity bounding boxes.
// ============================================================
void Soldier::resolveBlockCollisions(DamagableEntity** blocks, int count) {
    if (blocks == nullptr || count == 0) return;

    // Use the actual world-space bounding box of the player (includes position).
    // Never hardcode magic pixel widths/heights — if the sprite scale or hitbox
    // changes this code stays correct automatically.
    IntRect pb = this->getBoundingBox();
    float playerLeft = static_cast<float>(pb.left);
    float playerRight = static_cast<float>(pb.left + pb.width);
    float playerTop = static_cast<float>(pb.top);
    float playerBottom = static_cast<float>(pb.top + pb.height);

    for (int i = 0; i < count; i++) {
        if (blocks[i] == nullptr) continue;
        if (!blocks[i]->isAlive() || !blocks[i]->getStatus()) continue;

        IntRect b = blocks[i]->getBoundingBox();  // world-space

        float blockLeft = static_cast<float>(b.left);
        float blockRight = static_cast<float>(b.left + b.width);
        float blockTop = static_cast<float>(b.top);
        float blockBottom = static_cast<float>(b.top + b.height);

        // No overlap — skip
        if (playerRight <= blockLeft || playerLeft >= blockRight ||
            playerBottom <= blockTop || playerTop >= blockBottom) continue;

        // Penetration depth on each axis
        float overlapLeft = playerRight - blockLeft;   // push player left
        float overlapRight = blockRight - playerLeft;  // push player right
        float overlapTop = playerBottom - blockTop;    // push player up (land on top)
        float overlapBottom = blockBottom - playerTop;   // push player down (hit ceiling)

        // WHY velocity bias?
        // Pure min-overlap can misfire when the player runs into the side of a block
        // that's also slightly below them — the vertical overlap is tiny so it wins,
        // snapping the player on top when they should be blocked sideways.
        // We bias toward vertical resolution only when actually moving vertically,
        // and toward horizontal resolution when moving horizontally. This lets the
        // player walk into block sides without riding up onto them, and land cleanly
        // when dropping or jumping onto the top.
        bool movingDown = (this->velocityY > 0.5f);
        bool movingUp = (this->velocityY < -0.5f);
        bool movingHoriz = (this->velocityX > 0.5f || this->velocityX < -0.5f);

        // Default: pure minimum overlap
        float minOverlap = overlapLeft;
        int   resolveDir = 1;  // 1=left 2=right 3=top(land) 4=bottom(ceiling)

        if (overlapRight < minOverlap) { minOverlap = overlapRight;  resolveDir = 2; }
        if (overlapTop < minOverlap) { minOverlap = overlapTop;    resolveDir = 3; }
        if (overlapBottom < minOverlap) { minOverlap = overlapBottom; resolveDir = 4; }

        // Bias: if moving horizontally and NOT falling, prefer side resolution
        // so the player is blocked by the block face rather than teleported on top.
        if (movingHoriz && !movingDown) {
            float sideOverlap = overlapLeft;
            int   sideDir = 1;
            if (overlapRight < sideOverlap) { sideOverlap = overlapRight; sideDir = 2; }
            // Only override if the side overlap is not drastically larger than vertical
            // (avoids picking a huge side push when a tiny vertical push would do)
            if (sideOverlap < overlapTop * 2.f) {
                minOverlap = sideOverlap;
                resolveDir = sideDir;
            }
        }

        // Apply resolution
        if (resolveDir == 1) {
            this->position.x -= minOverlap;
            this->velocityX = 0.f;
        }
        else if (resolveDir == 2) {
            this->position.x += minOverlap;
            this->velocityX = 0.f;
        }
        else if (resolveDir == 3) {
            // Land on top of block — snap to exact surface
            this->position.y -= minOverlap;
            this->velocityY = 0.f;
            this->onGround = true;
        }
        else if (resolveDir == 4) {
            // Bonk head on underside of block
            this->position.y += minOverlap;
            if (this->velocityY < 0.f) this->velocityY = 0.f;
        }

        // Recompute player box after each resolution so multi-block situations
        // don't accumulate stale coordinates.
        pb = this->getBoundingBox();
        playerLeft = static_cast<float>(pb.left);
        playerRight = static_cast<float>(pb.left + pb.width);
        playerTop = static_cast<float>(pb.top);
        playerBottom = static_cast<float>(pb.top + pb.height);
    }
}