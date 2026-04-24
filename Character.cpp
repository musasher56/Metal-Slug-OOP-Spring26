#include "Character.h"

Character::Character()
    : position(100.f, 100.f), velocity(0.f, 0.f), texture(nullptr),
      onGround(false), gravity(1200.f), jumpForce(-500.f), moveSpeed(250.f),
      direction(1), level(nullptr) {

    // Allocate and load sprite texture (WHY: manual allocation avoids STL defaults)
    this->texture = new sf::Texture();
    if (!this->texture->loadFromFile("resources/Sprites/characters/marco_idle.png")) {
        delete this->texture;
        this->texture = nullptr;
        // Fallback: generate a solid cyan block if asset is missing
        sf::Image fallback;
        fallback.create(32, 48, sf::Color::Cyan);
        this->texture = new sf::Texture();
        this->texture->loadFromImage(fallback);
    }
    this->sprite.setTexture(*this->texture);
    this->sprite.setPosition(this->position);
}

Character::~Character() {
    if (this->texture != nullptr) {
        delete this->texture;
        this->texture = nullptr;
    }
}

// Deep copy: reload texture to guarantee independent memory (WHY: SFML ref-counts internally)
Character::Character(const Character& other)
    : position(other.position), velocity(other.velocity), texture(nullptr),
      onGround(other.onGround), gravity(other.gravity), jumpForce(other.jumpForce),
      moveSpeed(other.moveSpeed), direction(other.direction), level(other.level) {
    if (other.texture != nullptr) {
        this->texture = new sf::Texture();
        this->texture->loadFromFile("resources/Sprites/characters/marco_idle.png");
        this->sprite.setTexture(*this->texture);
        this->sprite.setPosition(this->position);
    }
}

Character& Character::operator=(const Character& other) {
    if (this != &other) {
        if (this->texture != nullptr) {
            delete this->texture;
            this->texture = nullptr;
        }
        this->position = other.position;
        this->velocity = other.velocity;
        this->onGround = other.onGround;
        this->gravity = other.gravity;
        this->jumpForce = other.jumpForce;
        this->moveSpeed = other.moveSpeed;
        this->direction = other.direction;
        this->level = other.level;

        if (other.texture != nullptr) {
            this->texture = new sf::Texture();
            this->texture->loadFromFile("resources/Sprites/characters/marco_idle.png");
            this->sprite.setTexture(*this->texture);
            this->sprite.setPosition(this->position);
        }
    }
    return *this;
}

void Character::update(float dt) {
    if (this->level == nullptr) return;
    this->applyPhysics(dt);
    this->resolveCollision();

    // Update sprite position and flip direction
    this->sprite.setPosition(this->position);
    if (this->direction == 0) this->sprite.setScale(-1.f, 1.f);
    else this->sprite.setScale(1.f, 1.f);
}

void Character::draw(sf::RenderWindow& window) {
    window.draw(this->sprite);
}

void Character::handleInput(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space && this->onGround) {
            this->velocity.y = this->jumpForce;
            this->onGround = false;
        }
    }
    if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
        bool pressed = (event.type == sf::Event::KeyPressed);
        if (event.key.code == sf::Keyboard::Left) {
            this->velocity.x = pressed ? -this->moveSpeed : 0.f;
            if (pressed) this->direction = 0;
        } else if (event.key.code == sf::Keyboard::Right) {
            this->velocity.x = pressed ? this->moveSpeed : 0.f;
            if (pressed) this->direction = 1;
        }
    }
}

void Character::applyPhysics(float dt) {
    this->velocity.y += this->gravity * dt;
    if (this->velocity.y > 800.f) this->velocity.y = 800.f;

    this->position.x += this->velocity.x * dt;
    this->position.y += this->velocity.y * dt;

    // Clamp to level boundaries (WHY: prevents falling off or clipping out of bounds)
    int maxX = this->level->getWidth() * this->level->getCellSize() - 32;
    if (this->position.x < 0.f) this->position.x = 0.f;
    if (this->position.x > (float)maxX) this->position.x = (float)maxX;
}

void Character::resolveCollision() {
    this->onGround = false;
    int charBottomRow = (int)((this->position.y + 48.f) / this->level->getCellSize());
    int leftCol = (int)(this->position.x / this->level->getCellSize());
    int rightCol = (int)((this->position.x + 31.f) / this->level->getCellSize());

    for (int col = leftCol; col <= rightCol; ++col) {
        if (this->level->isSolid(charBottomRow, col)) {
            this->position.y = (charBottomRow * this->level->getCellSize()) - 48.f;
            this->velocity.y = 0.f;
            this->onGround = true;
            break;
        }
    }
}

void Character::setLevel(const Level* lvl) { this->level = lvl; }
sf::Vector2f Character::getPosition() const { return this->position; }
sf::FloatRect Character::getBounds() const { return this->sprite.getGlobalBounds(); }

void Character::setPosition(float x, float y) {
    this->position.x = x;
    this->position.y = y;
    this->sprite.setPosition(this->position);
}