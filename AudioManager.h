#pragma once
#include "Constants.h"

// WHY: Stub AudioManager to avoid SFML audio dependency (OpenAL runtime issue on macOS)
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    void* getSound(const char* filename);
    bool loadSound(const char* filename);
    void applyLowPassFilter(void* sound);
    void setLowPassActive(bool val);
    bool isLowPassActive() const;

private:
    int soundCount;
    bool lowpassActive;
    int findSlot(const char* filename) const;

    char names[MAX_SOUNDS][MAX_NAME_LEN];
    bool loaded[MAX_SOUNDS];
};
