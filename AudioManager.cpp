#include "AudioManager.h"
#include <cstdio>















const char* AudioManager::MUSIC_TRACK_PATHS[AudioManager::NUM_MUSIC_TRACKS] = {
    "resources/Audio/title_theme.ogg",
    "resources/Audio/level1_music.ogg",
    "resources/Audio/level2_music.ogg",
    "resources/Audio/level3_music.ogg"
};





AudioManager::AudioManager()
    : currentTrack(-1)
    , musicPlaying(false)
    , musicVolume(80.f)
    , lowpassActive(false)
    , soundCount(0)
{
    for (int i = 0; i < MAX_SOUNDS; i++) {
        loaded[i] = false;
        names[i][0] = '\0';
    }
    this->music.setVolume(this->musicVolume);
    printf("[AudioManager] Initialized (music system active, %d tracks)\n",
           NUM_MUSIC_TRACKS);
}

AudioManager::~AudioManager() {
    
    this->stopMusic();
}





bool AudioManager::playMusicTrack(int trackIndex, bool loop) {
    if (trackIndex < 0 || trackIndex >= NUM_MUSIC_TRACKS) {
        printf("[AudioManager] Invalid track index: %d\n", trackIndex);
        return false;
    }

    
    if (this->currentTrack == trackIndex && this->musicPlaying) {
        return true;
    }

    
    this->stopMusic();

    
    const char* path = MUSIC_TRACK_PATHS[trackIndex];
    if (!this->music.openFromFile(path)) {
        printf("[AudioManager] Failed to open music: %s\n", path);
        this->currentTrack = -1;
        this->musicPlaying = false;
        return false;
    }

    this->music.setLoop(loop);
    this->music.setVolume(this->musicVolume);
    this->music.play();
    this->currentTrack = trackIndex;
    this->musicPlaying = true;

    printf("[AudioManager] Now playing track %d: %s (loop=%s)\n",
           trackIndex, path, loop ? "true" : "false");
    return true;
}

void AudioManager::stopMusic() {
    if (this->currentTrack >= 0 || this->musicPlaying) {
        this->music.stop();
        
        
        
        this->music.openFromFile("");
        this->currentTrack = -1;
        this->musicPlaying = false;
        printf("[AudioManager] Music stopped and handle released\n");
    }
}

void AudioManager::pauseMusic() {
    if (this->musicPlaying) {
        this->music.pause();
        this->musicPlaying = false;
        
    }
}

void AudioManager::resumeMusic() {
    if (this->currentTrack >= 0 && !this->musicPlaying) {
        this->music.play();
        this->musicPlaying = true;
    }
}

int AudioManager::getCurrentTrackIndex() const {
    return this->currentTrack;
}

bool AudioManager::isMusicPlaying() const {
    return this->musicPlaying;
}





void AudioManager::setMusicVolume(float volume) {
    if (volume < 0.f)   volume = 0.f;
    if (volume > 100.f) volume = 100.f;
    this->musicVolume = volume;
    this->music.setVolume(volume);
}

float AudioManager::getMusicVolume() const {
    return this->musicVolume;
}





inline int strLen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

inline int strCompare(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

bool AudioManager::loadSound(const char* filename) {
    printf("[AudioManager] loadSound stub: %s (not implemented yet)\n", filename);
    return false;
}

void* AudioManager::getSound(const char* filename) {
    return nullptr;
}

void AudioManager::applyLowPassFilter(void* sound) {
    
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