#pragma once
#include "Constants.h"



class PerlinNoise {
private:
    int permutation[512];  
    int seed;

    static const float GRAD_TABLE[8];

public:
    PerlinNoise(int seed);

    float noise(float x, float y);

    float fade(float t);

    float lerp(float t, float a, float b);

    float grad(int hash, float x, float y);

    static float fractal(float x, float amplitude, float frequency,
        float persistence, int harmonics, int seed);
};


class NoiseProfile;  

class FractalNoise {
private:
    PerlinNoise* perlin;
    float octaves;        
    float persistence;  
    float lacunarity;     
    float amplitude;      
    float frequency;     

public:
    FractalNoise(PerlinNoise* perlin);
    ~FractalNoise();

    float sample(float x, float y);

    void generateHeightMap(int width, int* out);

    int mapToBiome(float height);

    void setProfile(NoiseProfile* profile);

    void setAmplitude(float a) { 
        this->amplitude = a; }
    void setFrequency(float f) {
        this->frequency = f; }
    void setPersistence(float p) {
        
       this->persistence = p; }
    void setLacunarity(float l) {
        
        this->lacunarity = l; }
    void setOctaves(float o) {
        this->octaves = o; }

    float getAmplitude()   const {
        return this->amplitude; }
    float getFrequency()   const {
        return this->frequency; }
    float getPersistence() const {
        return this->persistence; }
    float getLacunarity()  const { 
        return this->lacunarity; }
    float getOctaves()     const {
        return this->octaves; }
};



class NoiseProfile {
protected:
    float amplitude;     
    float frequency;     
    float persistence;  
    float lacunarity;   
    float octaves;      
    int type;           
    int seed;            

public:
    NoiseProfile();
    virtual ~NoiseProfile();

    virtual void applyProfile(FractalNoise* noise) = 0;

    virtual int getType() = 0;

    static NoiseProfile* create(int type);

    float getAmplitude()   const {
        return this->amplitude; }
    float getFrequency()   const {
        return this->frequency; }
    float getPersistence() const {
        return this->persistence; }
    float getLacunarity()  const {
        return this->lacunarity; }
    float getOctaves()     const {
        return this->octaves; }

    int   getHarmonics()   const {
        return static_cast<int>(this->octaves); }

    int   getSeed()        const { 
        return this->seed; }
    int   getTypeValue()   const {
        return this->type; }

    void  setSeed(int s) { this->seed = s; }
};



class AmplifiedProfile : public NoiseProfile {
public:
    AmplifiedProfile();
    virtual void applyProfile(FractalNoise* noise);
    virtual int getType();
};


class FlatProfile : public NoiseProfile {
public:
    FlatProfile();
    virtual void applyProfile(FractalNoise* noise);
    virtual int getType();
};


class NormalProfile : public NoiseProfile {
public:
    NormalProfile();
    virtual void applyProfile(FractalNoise* noise);
    virtual int getType();
};
