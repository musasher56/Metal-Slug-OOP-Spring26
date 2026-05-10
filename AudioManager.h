#pragma once
#include "Constants.h"
#include <SFML/Audio.hpp>




























class AudioManager {
public:
    static const int NUM_MUSIC_TRACKS = 4;

    AudioManager();
    ~AudioManager();

    

    
    
    
    bool playMusicTrack(int trackIndex, bool loop = true);

    
    void stopMusic();

    
    void pauseMusic();
    void resumeMusic();

    
    int  getCurrentTrackIndex() const;

    
    bool isMusicPlaying() const;

    
    void setMusicVolume(float volume);   
    float getMusicVolume() const;

    
    void* getSound(const char* filename);
    bool  loadSound(const char* filename);
    void  applyLowPassFilter(void* sound);
    void  setLowPassActive(bool val);
    bool  isLowPassActive() const;

private:
    
    sf::Music music;
    int       currentTrack;       
    bool      musicPlaying;
    float     musicVolume;

    bool      lowpassActive;

    
    int       soundCount;
    int       findSlot(const char* filename) const;
    char      names[MAX_SOUNDS][MAX_NAME_LEN];
    bool      loaded[MAX_SOUNDS];

    
    
    static const char* MUSIC_TRACK_PATHS[NUM_MUSIC_TRACKS];
};