#include "AudioManager.h"
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
// Placeholder music file paths.
//
// *** CREATE YOUR .ogg FILES WITH THESE EXACT NAMES ***
//
//   resources/Audio/title_theme.ogg    — plays on title screen & char select
//   resources/Audio/level1_music.ogg   — Level 1 "Ruins" background music
//   resources/Audio/level2_music.ogg   — Level 2 "Cold Death" background music
//   resources/Audio/level3_music.ogg   — Level 3 "Blasphemous City" background music
//
// Recommended format: OGG Vorbis, 44100 Hz, stereo.
// The files must be in the working directory alongside the executable,
// or adjust the paths below to match your layout.
// ─────────────────────────────────────────────────────────────────────────────
const char* AudioManager::MUSIC_TRACK_PATHS[AudioManager::NUM_MUSIC_TRACKS] = {
    "resources/Audio/title_theme.ogg",
    "resources/Audio/level1_music.ogg",
    "resources/Audio/level2_music.ogg",
    "resources/Audio/level3_music.ogg"
};

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

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
    // Guarantee the music handle is released on destruction — no leaks.
    this->stopMusic();
}

// ─────────────────────────────────────────────────────────────────────────────
// Music playback
// ─────────────────────────────────────────────────────────────────────────────

bool AudioManager::playMusicTrack(int trackIndex, bool loop) {
    if (trackIndex < 0 || trackIndex >= NUM_MUSIC_TRACKS) {
        printf("[AudioManager] Invalid track index: %d\n", trackIndex);
        return false;
    }

    // If the SAME track is already playing, do nothing — avoids restart pops.
    if (this->currentTrack == trackIndex && this->musicPlaying) {
        return true;
    }

    // Stop whatever is currently playing and release the file handle.
    this->stopMusic();

    // Open the new track.
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
        // Release the file handle by opening an empty string.
        // sf::Music::openFromFile("") will fail silently but releases the
        // previous file's handle, preventing OS-level file locks.
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
        // currentTrack is preserved so resumeMusic() can restart it.
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

// ─────────────────────────────────────────────────────────────────────────────
// Volume
// ─────────────────────────────────────────────────────────────────────────────

void AudioManager::setMusicVolume(float volume) {
    if (volume < 0.f)   volume = 0.f;
    if (volume > 100.f) volume = 100.f;
    this->musicVolume = volume;
    this->music.setVolume(volume);
}

float AudioManager::getMusicVolume() const {
    return this->musicVolume;
}

// ─────────────────────────────────────────────────────────────────────────────
// Legacy sound-effect stubs (kept for API compatibility)
// ─────────────────────────────────────────────────────────────────────────────

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
    // No-op placeholder for future SFX filtering
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
