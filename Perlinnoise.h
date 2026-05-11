#pragma once
#include "Constants.h"

// ─────────────────────────────────────────────────────────────────────────────
// PerlinNoise.h — ALL noise/terrain-profile classes in ONE file
//
// Contains:
//   1. PerlinNoise      — seeded noise generator (permutation table + interpolation)
//   2. FractalNoise     — layers PerlinNoise octaves → produces heightmap for dirt blocks
//   3. NoiseProfile     — abstract base for terrain profiles (Factory pattern)
//   4. AmplifiedProfile — big dramatic mountains
//   5. FlatProfile      — gentle rolling terrain
//   6. NormalProfile    — balanced playable terrain
//
// Factory Pattern: NoiseProfile::create(type) uses function pointer array
//   (NO switch statement — avoids P5 penalty)
//
// TWO WAYS TO USE:
//
// 1. Instance-based (for survival mode with BlockManager):
//    PerlinNoise perlin(seed);
//    FractalNoise fractal(&perlin);
//    NoiseProfile* profile = NoiseProfile::create(NOISE_AMPLIFIED);
//    fractal.setProfile(profile);
//    int* heightmap = new int[levelWidth];
//    fractal.generateHeightMap(levelWidth, heightmap);
//    blockManager->buildProceduralTerrain(0.f, surfaceY, heightmap, levelWidth);
//    delete[] heightmap;
//    delete profile;
//
// 2. Static method (for campaign mode Level::generateColumn):
//    float h = PerlinNoise::fractal(worldX, amplitude, frequency,
//                                   persistence, harmonics, seed);
//    // h is in [0, 1] — directly use as terrain height
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// PerlinNoise — seeded 2D noise generator
//
// UML: PerlinNoise (C) with permutation[], seed, noise(x,y), fade, lerp, grad
//
// How it works:
//   1. Seed generates a shuffled permutation table (0-255, doubled for wrapping)
//   2. noise(x, y) looks up 4 grid-corner gradients, interpolates with fade
//   3. Output is in range [-1, +1], typically mapped to [0, 1] by the caller
//
// Also provides a static fractal() method for campaign mode Level generation.
// ─────────────────────────────────────────────────────────────────────────────

class PerlinNoise {
private:
    int permutation[512];   // doubled permutation table for overflow-free access
    int seed;

    // Gradient lookup table — replaces switch statement in grad()
    // 4 gradient directions: (1,1), (-1,1), (1,-1), (-1,-1)
    // Stored as [gx0, gy0, gx1, gy1, gx2, gy2, gx3, gy3]
    static const float GRAD_TABLE[8];

public:
    PerlinNoise(int seed);

    // Returns noise value in [-1, +1] for given coordinates
    float noise(float x, float y);

    // Smooth interpolation helper (6t^5 - 15t^4 + 10t^3)
    float fade(float t);

    // Linear interpolation: a + t*(b-a)
    float lerp(float t, float a, float b);

    // Dot product of gradient vector and distance vector
    // Uses GRAD_TABLE array lookup (NO switch statement)
    float grad(int hash, float x, float y);

    // ── Static fractal noise — for campaign mode Level::generateColumn ──
    //
    // Convenience method that creates a cached PerlinNoise instance internally
    // and layers multiple octaves. Returns value in [0, 1].
    //
    // Parameters:
    //   x           — world X position to sample
    //   amplitude   — controls terrain height (doesn't change [0,1] output,
    //                 but affects how profiles are applied internally)
    //   frequency   — hill width (lower = wider hills)
    //   persistence — how much each octave contributes (0.3-0.6 typical)
    //   harmonics   — number of noise layers (same as octaves)
    //   seed        — permutation table seed (same seed = same terrain)
    static float fractal(float x, float amplitude, float frequency,
        float persistence, int harmonics, int seed);
};


// ─────────────────────────────────────────────────────────────────────────────
// FractalNoise — generates terrain heightmaps from PerlinNoise
//
// UML: FractalNoise (C) with perlin, octaves, persistence, lacunarity,
//      amplitude, frequency. Methods: sample(), generateHeightMap(),
//      mapToBiome(), setProfile()
//
// How it works:
//   1. PerlinNoise gives you smooth random values per coordinate
//   2. FractalNoise layers (octaves) of noise at different scales
//   3. generateHeightMap() samples across your level width and produces
//      an int array where each value = number of dirt blocks to stack
//   4. That int array feeds directly into BlockManager to build terrain
//
// Default: 2 octaves (smooth rolling hills, not noisy spiky garbage).
// Parameters tuned to produce BLOCK heights (0-25 range).
// ─────────────────────────────────────────────────────────────────────────────

class NoiseProfile;  // forward declaration

class FractalNoise {
private:
    PerlinNoise* perlin;
    float octaves;        // how many layers of noise (default: 2)
    float persistence;    // how much each octave contributes (default: 0.5)
    float lacunarity;     // how much frequency increases per octave (default: 2.0)
    float amplitude;      // max terrain height in blocks (default: 15)
    float frequency;      // how wide the hills are (default: 0.02 — big rolling hills)

public:
    FractalNoise(PerlinNoise* perlin);
    ~FractalNoise();

    // Sample fractal noise at (x, y). Returns value in [0, 1]
    float sample(float x, float y);

    // Generate a 1D heightmap for terrain.
    // out[] is filled with integer heights (number of blocks per column).
    // width = number of columns to generate.
    // Each column is one MountainBlock wide (48px).
    void generateHeightMap(int width, int* out);

    // Map a normalized height [0,1] to a biome constant (BIOME_AERIAL/PLAINS/AQUATIC)
    int mapToBiome(float height);

    // Apply a NoiseProfile to set amplitude/frequency/etc.
    void setProfile(NoiseProfile* profile);

    // ── Parameter setters (used by NoiseProfile::applyProfile) ──
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
