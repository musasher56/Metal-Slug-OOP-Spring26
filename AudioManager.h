#pragma once
#include "Constants.h"

// WHY: Stub AudioManager to avoid SFML audio dependency (OpenAL runtime issue on macOS)
// This allows the menu to work without sound. Audio can be re-enabled later.

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Stub methods — return safe defaults, do nothing
    void* getSound(const char* filename);
    bool loadSound(const char* filename);
    void applyLowPassFilter(void* sound);
    void setLowPassActive(bool val);
    bool isLowPassActive() const;

private:
    int soundCount;
    bool lowpassActive;
    int findSlot(const char* filename) const;

    // Stub storage — no SFML types
    char names[MAX_SOUNDS][MAX_NAME_LEN];
    bool loaded[MAX_SOUNDS];
};