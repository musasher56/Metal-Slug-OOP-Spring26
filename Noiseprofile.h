#pragma once

// =============================================================================
// NoiseProfile.h
//
// Abstract base class for terrain generation parameter bundles.
// "Injecting OOP into the veins of Perlin Noise" — project spec.
//
// The math lives in PerlinNoise. This class purely holds configuration:
// amplitude, frequency, persistence, harmonics, and a domain seed.
// PerlinNoise::fractal() takes a NoiseProfile* and reads these at runtime,
// enabling polymorphic terrain selection without any type-checking.
//
// DOMAIN SEED:
//   Each concrete profile carries a hardcoded seed integer. This shifts
//   the entire noise domain so that Normal, Amplified, and Flat profiles
//   don't just look like rescaled versions of each other — they sample
//   from genuinely different regions of the gradient field. Two groups
//   running the same code will get the same terrain (deterministic), but
//   the three profiles will look qualitatively distinct from each other.
//
// FACTORY:
//   NoiseProfile::create(int type) uses a static function pointer array.
//   NO switch statement — P5 penalty avoidance. Adding a 4th profile
//   requires only appending one lambda to the array in NoiseProfile.cpp.
//   The dispatch logic itself never changes (Open/Closed Principle).
//
// MEMORY NOTE:
//   create() returns a heap-allocated object. The caller owns it and must
//   delete it. NoiseProfile has a virtual destructor to ensure correct
//   cleanup when deleting through a base pointer.
// =============================================================================
class NoiseProfile {
public:

    // Virtual destructor: mandatory for any polymorphic base.
    // Without this, deleting a NoiseProfile* that points to an
    // AmplifiedProfile would only call ~NoiseProfile, leaking any
    // resources in the derived class. (P3 penalty prevention.)
    virtual ~NoiseProfile() {}

    // How tall peaks are relative to level height.
    // Range [0.0, 1.0] — fed as 'amplitude' into PerlinNoise::fractal().
    virtual float getAmplitude()   const = 0;

    // Horizontal compression of terrain features.
    // Higher value = features are closer together horizontally.
    // Fed as 'frequency' into PerlinNoise::fractal().
    virtual float getFrequency()   const = 0;

    // Controls how quickly harmonic amplitude decays with each layer.
    // 0.0 = only the first harmonic contributes (very smooth).
    // 1.0 = all harmonics equally weighted (very rough).
    // Fed as 'persistence' into PerlinNoise::fractal().
    virtual float getPersistence() const = 0;

    // Number of harmonic (noise) layers to stack.
    // More harmonics = more terrain detail = more computation.
    // Capped at 6 inside PerlinNoise::fractal() regardless.
    virtual int   getHarmonics()   const = 0;

    // Domain seed: shifts the noise sampling region.
    // Different seeds → different terrain shape even at same parameters.
    // Each profile has a distinct hardcoded seed so the three modes
    // look qualitatively different by design.
    virtual int   getSeed()        const = 0;

    // -------------------------------------------------------------------------
    // Factory method.
    // Implemented in NoiseProfile.cpp via a static function pointer array.
    // Maps: NOISE_AMPLIFIED (0) → AmplifiedProfile
    //       NOISE_FLAT      (1) → FlatProfile
    //       NOISE_NORMAL    (2) → NormalProfile
    // (Constants from Constants.h)
    // Returns heap-allocated profile. Caller must delete.
    // -------------------------------------------------------------------------
    static NoiseProfile* create(int type);
};


// =============================================================================
// NormalProfile
// The spec description: "normal where the depth of the ocean and the altitude
// of the peaks is uniformly distributed."
// Mid-range everything — the baseline for comparison.
// =============================================================================
class NormalProfile : public NoiseProfile {
public:
    // Moderate amplitude: peaks reach roughly 55% of level height
    float getAmplitude()   const { return 0.55f;  }

    // Frequency 0.030: one terrain "wave" every ~33 columns (1584px).
    // The visible screen shows 26 columns, so ~0.8 of a full cycle per screen
    // — enough to see a peak AND a valley simultaneously. At 0.018 the cycle
    // was 55 columns, producing subtler variation that looked flat on a demo screen.
    float getFrequency()   const { return 0.030f; }

    // Standard 50% persistence: each harmonic is half the previous
    float getPersistence() const { return 0.50f;  }

    // 4 harmonics: good balance of detail vs computation
    int   getHarmonics()   const { return 4;      }

    // Seed chosen to center the terrain profile visually
    int   getSeed()        const { return 9371;   }
};


// =============================================================================
// AmplifiedProfile
// The spec description: "peaks and ocean are extra amplified, plains remain same."
// Higher amplitude pushes extremes further apart. Higher persistence adds
// jagged detail to peaks (more harmonics, higher contribution per layer).
// Frequency is slightly higher to produce tighter, more dramatic ridgelines.
// =============================================================================
class AmplifiedProfile : public NoiseProfile {
public:
    // High amplitude: dramatic tall mountains and deep ocean trenches
    float getAmplitude()   const { return 0.82f;  }

    // Slightly higher frequency: ridges are closer together, more jagged
    float getFrequency()   const { return 0.024f; }

    // Higher persistence: each harmonic is louder, adding roughness to peaks
    float getPersistence() const { return 0.62f;  }

    // 5 harmonics: extra detail layer for the dramatic landscape
    int   getHarmonics()   const { return 5;      }

    // Distinct seed: samples a different noise region than Normal
    int   getSeed()        const { return 17239;  }
};


// =============================================================================
// FlatProfile
// The spec description: "opposite of Amplified, no dramatic peaks and deep oceans."
// Low amplitude keeps everything near the middle of the height range.
// Low frequency means features are widely spaced (broad gentle hills).
// Low persistence means only the smoothest (lowest-frequency) layer matters.
// =============================================================================
class FlatProfile : public NoiseProfile {
public:
    // Low amplitude: terrain stays near the middle height — gentle slopes
    float getAmplitude()   const { return 0.22f;  }

    // Low frequency: features are far apart horizontally (broad plains)
    float getFrequency()   const { return 0.009f; }

    // Low persistence: higher harmonics barely contribute, keeps it smooth
    float getPersistence() const { return 0.38f;  }

    // 3 harmonics: fewer layers = smoother, flatter result
    int   getHarmonics()   const { return 3;      }

    // Distinct seed: samples a different region of the gradient field
    int   getSeed()        const { return 5417;   }
};