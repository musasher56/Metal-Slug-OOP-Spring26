#include "PerlinNoise.h"
#include <cmath>

const float PerlinNoise::GRAD_TABLE[8] = {
    1.0f,  1.0f, 
   -1.0f,  1.0f,   
    1.0f, -1.0f,   
   -1.0f, -1.0f    
};


PerlinNoise::PerlinNoise(int seed)
    : seed(seed)
{
    for (int i = 0; i < 256; i++) {
        this->permutation[i] = i;
    }

    int rng = seed;
    if (rng == 0) rng = 1; 

    for (int i = 255; i > 0; i--) {
 
        rng = (rng * 1103515245 + 12345) & 0x7FFFFFFF;
        int j = rng % (i + 1);
        if (j < 0) j = -j;

        int tmp = this->permutation[i];
        this->permutation[i] = this->permutation[j];
        this->permutation[j] = tmp;
    }

    for (int i = 0; i < 256; i++) {
        this->permutation[256 + i] = this->permutation[i];
    }
}


float PerlinNoise::fade(float t) {

    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}


float PerlinNoise::lerp(float t, float a, float b) {
    return a + t * (b - a);
}


float PerlinNoise::grad(int hash, float x, float y) {
    int h = hash & 3;
    // Look up gradient (gx, gy) from the table
    float gx = GRAD_TABLE[h * 2];     
    float gy = GRAD_TABLE[h * 2 + 1];  
    return gx * x + gy * y;
}


float PerlinNoise::noise(float x, float y) {
 
    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;

    float xf = x - floorf(x);
    float yf = y - floorf(y);

    float u = this->fade(xf);
    float v = this->fade(yf);

    int p00 = this->permutation[this->permutation[xi] + yi];
    int p10 = this->permutation[this->permutation[xi + 1] + yi];
    int p01 = this->permutation[this->permutation[xi] + yi + 1];
    int p11 = this->permutation[this->permutation[xi + 1] + yi + 1];

    float g00 = this->grad(p00, xf, yf);
    float g10 = this->grad(p10, xf - 1, yf);
    float g01 = this->grad(p01, xf, yf - 1);
    float g11 = this->grad(p11, xf - 1, yf - 1);

    float x0 = this->lerp(u, g00, g10); 
    float x1 = this->lerp(u, g01, g11);  
    float result = this->lerp(v, x0, x1);

    return result;
}



float PerlinNoise::fractal(float x, float amplitude, float frequency,
    float persistence, int harmonics, int seed)
{
    static PerlinNoise* s_cachedPerlin = nullptr;
    static int s_cachedSeed = 0x7FFFFFFF;  

    if (s_cachedPerlin == nullptr || s_cachedSeed != seed) {
        if (s_cachedPerlin != nullptr) {
            delete s_cachedPerlin;
        }
        s_cachedPerlin = new PerlinNoise(seed);
        s_cachedSeed = seed;
    }
    int numOctaves = harmonics;
    if (numOctaves < 1) numOctaves = 1;
    if (numOctaves > 6) numOctaves = 6;

    float total = 0.0f;
    float currentAmplitude = 1.0f;
    float currentFrequency = frequency;
    float maxAmplitude = 0.0f;
    float lacunarity = 2.0f;  

    for (int i = 0; i < numOctaves; i++) {
        float noiseVal = s_cachedPerlin->noise(x * currentFrequency, 0.0f);
        noiseVal = (noiseVal + 1.0f) * 0.5f; 

        total += noiseVal * currentAmplitude;
        maxAmplitude += currentAmplitude;

        currentAmplitude *= persistence;
        currentFrequency *= lacunarity;
    }

    if (maxAmplitude > 0.0f) {
        total /= maxAmplitude;
    }

    (void)amplitude; 

    return total;
}


FractalNoise::FractalNoise(PerlinNoise* perlin)
    : perlin(perlin)
    , octaves(2.0f)           
    , persistence(0.5f)       
    , lacunarity(2.0f)      
    , amplitude(15.0f)      
    , frequency(0.02f)        
{
}

FractalNoise::~FractalNoise() {
   
    this->perlin = nullptr;
}


float FractalNoise::sample(float x, float y) {
    if (this->perlin == nullptr) return 0.0f;

    float total = 0.0f;
    float currentAmplitude = 1.0f;
    float currentFrequency = this->frequency;
    float maxAmplitude = 0.0f;

    int numOctaves = (int)this->octaves;
    if (numOctaves < 1) numOctaves = 1;
    if (numOctaves > 6) numOctaves = 6;

    for (int i = 0; i < numOctaves; i++) {
    
        float noiseVal = this->perlin->noise(x * currentFrequency, y * currentFrequency);
        noiseVal = (noiseVal + 1.0f) * 0.5f; 

        total += noiseVal * currentAmplitude;
        maxAmplitude += currentAmplitude;

        currentAmplitude *= this->persistence;
        currentFrequency *= this->lacunarity;
    }

    if (maxAmplitude > 0.0f) {
        total /= maxAmplitude;
    }

    return total;
}


void FractalNoise::generateHeightMap(int width, int* out) {
    if (out == nullptr || width <= 0) return;

    int maxBlocks = (int)this->amplitude;
    if (maxBlocks < 1) maxBlocks = 1;
    if (maxBlocks > 30) maxBlocks = 30;

    int minBlocks = 1;

    for (int col = 0; col < width; col++) {
     
        float noiseVal = this->sample((float)col, 0.0f);

        int height = minBlocks + (int)(noiseVal * (float)(maxBlocks - minBlocks));

        if (height < minBlocks) height = minBlocks;
        if (height > maxBlocks) height = maxBlocks;

        out[col] = height;
    }
}


int FractalNoise::mapToBiome(float height) {
    if (height > BIOME_AERIAL_THRESHOLD) {
        return BIOME_AERIAL;   
    }
    else if (height < BIOME_AQUATIC_THRESHOLD) {
        return BIOME_AQUATIC;  
    }
    else {
        return BIOME_PLAINS;    
    }
}


void FractalNoise::setProfile(NoiseProfile* profile) {
    if (profile == nullptr) return;
    profile->applyProfile(this);
}


NoiseProfile::NoiseProfile()
    : amplitude(15.0f)
    , frequency(0.02f)
    , persistence(0.5f)
    , lacunarity(2.0f)
    , octaves(2.0f)
    , type(NOISE_NORMAL)
    , seed(42)
{
}

NoiseProfile::~NoiseProfile() {
   
}

static NoiseProfile* createAmplified() {
    return new AmplifiedProfile(); }
static NoiseProfile* createFlat() {
    return new FlatProfile(); }
static NoiseProfile* createNormal() {
    return new NormalProfile(); }

NoiseProfile* NoiseProfile::create(int type) {
    typedef NoiseProfile* (*CreatorFn)();
    static CreatorFn creators[3] = {
        createAmplified,
        createFlat,      
        createNormal    
    };

    if (type < 0 || type >= 3) {
        type = NOISE_NORMAL;
    }

    return creators[type]();
}

AmplifiedProfile::AmplifiedProfile() {
    this->amplitude = 25.0f;    
    this->frequency = 0.015f;
    this->persistence = 0.55f;  
    this->lacunarity = 2.0f;    
    this->octaves = 3.0f;     
    this->type = NOISE_AMPLIFIED;
    this->seed = 42;
}

void AmplifiedProfile::applyProfile(FractalNoise* noise) {
    if (noise == nullptr) return;
    noise->setAmplitude(this->amplitude);
    noise->setFrequency(this->frequency);
    noise->setPersistence(this->persistence);
    noise->setLacunarity(this->lacunarity);
    noise->setOctaves(this->octaves);
}

int AmplifiedProfile::getType() {
    return NOISE_AMPLIFIED;
}


FlatProfile::FlatProfile() {
    this->amplitude = 3.0f;     
    this->frequency = 0.05f;  
    this->persistence = 0.3f;     
    this->lacunarity = 2.0f;     
    this->octaves = 1.0f;     
    this->type = NOISE_FLAT;
    this->seed = 42;
}

void FlatProfile::applyProfile(FractalNoise* noise) {
    if (noise == nullptr) return;
    noise->setAmplitude(this->amplitude);
    noise->setFrequency(this->frequency);
    noise->setPersistence(this->persistence);
    noise->setLacunarity(this->lacunarity);
    noise->setOctaves(this->octaves);
}

int FlatProfile::getType() {
    return NOISE_FLAT;
}



NormalProfile::NormalProfile() {
    this->amplitude = 15.0f;    
    this->frequency = 0.02f;    
    this->persistence = 0.5f;    
    this->lacunarity = 2.0f;  
    this->octaves = 2.0f;     
    this->type = NOISE_NORMAL;
    this->seed = 42;
}

void NormalProfile::applyProfile(FractalNoise* noise) {
    if (noise == nullptr) return;
    noise->setAmplitude(this->amplitude);
    noise->setFrequency(this->frequency);
    noise->setPersistence(this->persistence);
    noise->setLacunarity(this->lacunarity);
    noise->setOctaves(this->octaves);
}

int NormalProfile::getType() {
    return NOISE_NORMAL;
}
