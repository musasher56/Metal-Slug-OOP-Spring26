#pragma once
#include "Constants.h"
#include <SFML/Audio.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// AudioManager — manages background music tracks and sound effects.
//
// Music tracks:
//   Track 0: Title screen / character selection
//   Track 1: Level 1 music
//   Track 2: Level 2 music
//   Track 3: Level 3 music
//
// Placeholder filenames (create your .ogg files with these exact names):
//   resources/Audio/title_theme.ogg
//   resources/Audio/level1_music.ogg
//   resources/Audio/level2_music.ogg
//   resources/Audio/level3_music.ogg
//
// Usage:
//   audMgr->playMusicTrack(0);   // start title music
//   audMgr->stopMusic();          // stop whatever is playing
//   audMgr->playMusicTrack(2);   // cross-fade into level 2
//
// Leak prevention:
//   - Only one sf::Music object is ever open at a time.
//   - stopMusic() calls music.stop() + music.openFromFile("") to release
//     the file handle immediately, preventing resource leaks.
//   - The destructor calls stopMusic() to guarantee cleanup.
// ─────────────────────────────────────────────────────────────────────────────

class AudioManager {
public:
    static const int NUM_MUSIC_TRACKS = 4;

    AudioManager();
    ~AudioManager();

    // ── Music ──────────────────────────────────────────────────────────────

    // Plays the given track (0-3).  If another track is already playing it
    // is stopped first (no overlapping).  Looping is ON by default.
    // Returns false if the file couldn't be opened.
    bool playMusicTrack(int trackIndex, bool loop = true);

    // Stops the current music track and releases the file handle.
    void stopMusic();

    // Pauses / resumes the current track without releasing the handle.
    void pauseMusic();
    void resumeMusic();

    // Returns the index of the currently playing track, or -1 if none.
    int  getCurrentTrackIndex() const;

    // Returns true if a track is currently playing (not paused, not stopped).
    bool isMusicPlaying() const;

    // ── Volume ─────────────────────────────────────────────────────────────
    void setMusicVolume(float volume);   // 0..100
    float getMusicVolume() const;

    // ── Sound effects (legacy stubs, kept for compatibility) ───────────────
    void* getSound(const char* filename);
    bool  loadSound(const char* filename);
    void  applyLowPassFilter(void* sound);
    void  setLowPassActive(bool val);
    bool  isLowPassActive() const;

private:
    // Single sf::Music instance — only one track can play at a time.
    sf::Music music;
    int       currentTrack;       // -1 = no track loaded
    bool      musicPlaying;
    float     musicVolume;

    bool      lowpassActive;

    // Sound effect slots (legacy, unused but kept for API compatibility)
    int       soundCount;
    int       findSlot(const char* filename) const;
    char      names[MAX_SOUNDS][MAX_NAME_LEN];
    bool      loaded[MAX_SOUNDS];

    // Placeholder file paths — replace the actual .ogg files on disk.
    // The code references these strings; you just create matching files.
    static const char* MUSIC_TRACK_PATHS[NUM_MUSIC_TRACKS];
};
