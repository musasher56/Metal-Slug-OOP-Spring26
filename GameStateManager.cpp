#include "GameStateManager.h"


GameStateManager::GameStateManager() 
    : stateStack(nullptr), stackSize(0), capacity(10) {
    this->stateStack = new GameState*[this->capacity];
    for (int i = 0; i < this->capacity; i++) {
        this->stateStack[i] = nullptr;
    }
}

GameStateManager::~GameStateManager() {
    
    while (this->stackSize > 0) {
        this->pop();
    }
    if (this->stateStack != nullptr) {
        delete[] this->stateStack;
        this->stateStack = nullptr;
    }
}

void GameStateManager::push(GameState* state) {
    if (state == nullptr) return;
    
    
    if (this->stackSize >= this->capacity) {
        int newCapacity = this->capacity * 2;
        GameState** newStack = new GameState*[newCapacity];
        for (int i = 0; i < this->capacity; i++) {
            newStack[i] = this->stateStack[i];
        }
        for (int i = this->capacity; i < newCapacity; i++) {
            newStack[i] = nullptr;
        }
        delete[] this->stateStack;
        this->stateStack = newStack;
        this->capacity = newCapacity;
    }
    
    this->stateStack[this->stackSize] = state;
    this->stackSize++;
    state->onEnter();
}

void GameStateManager::pop() {
    if (this->stackSize <= 0) return;
    
    this->stackSize--;
    if (this->stateStack[this->stackSize] != nullptr) {
        this->stateStack[this->stackSize]->onExit();
        delete this->stateStack[this->stackSize];
        this->stateStack[this->stackSize] = nullptr;
    }
}

GameState* GameStateManager::peek() {
    if (this->stackSize <= 0) return nullptr;
    return this->stateStack[this->stackSize - 1];
}

void GameStateManager::update(float dt) {
    GameState* top = this->peek();
    if (top != nullptr) {
        top->update(dt);
    }
}

void GameStateManager::render(RenderWindow& window) {
    GameState* top = this->peek();
    if (top != nullptr) {
        top->render(window);
    }
}

void GameStateManager::handleEvent(Event& event) {
    GameState* top = this->peek();
    if (top != nullptr) {
        top->handleEvent(event);
    }
}

void GameStateManager::changeState(GameState* newState) {
    if (newState == nullptr) return;
    
    
    if (this->stackSize > 0) {
        this->pop();
    }
    this->push(newState);
}
