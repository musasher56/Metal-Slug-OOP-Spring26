#include "AudioManager.h"
#include <cstdio>

// WHY: Manual string helper to avoid <cstring> (constraint compliance)
inline int strLen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

inline void strCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

inline int strCompare(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

AudioManager::AudioManager() : soundCount(0), lowpassActive(false) {
    for (int i = 0; i < MAX_SOUNDS; i++) {
        loaded[i] = false;
        names[i][0] = '\0';
    }
    printf("[AudioManager] Stub initialized (audio disabled)\n");
}

AudioManager::~AudioManager() {}

bool AudioManager::loadSound(const char* filename) {
    // WHY: Stub — always return false to indicate audio unavailable
    printf("[AudioManager] loadSound stub: %s (disabled)\n", filename);
    return false;
}

void* AudioManager::getSound(const char* filename) {
    // WHY: Stub — return nullptr instead of sf::Sound&
    return nullptr;
}

void AudioManager::applyLowPassFilter(void* sound) {
    // WHY: Stub — no-op
}

void AudioManager::setLowPassActive(bool val) {
    this->lowpassActive = val;
}

bool AudioManager::isLowPassActive() const {
    return this->lowpassActive;
}

int AudioManager::findSlot(const char* filename) const {
    for (int i = 0; i < this->soundCount; i++) {
        if (!this->loaded[i]) continue;
        if (strCompare(this->names[i], filename) == 0) {
            return i;
        }
    }
    return -1;
}