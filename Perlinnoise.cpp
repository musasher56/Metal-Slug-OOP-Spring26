#include "PerlinNoise.h"
#include <cmath>   // floorf

// =============================================================================
// Per-harmonic seed offsets.
// Drawn from a prime-gap sequence so no two harmonics share frequency aliases.
// Adding these to the profile's base seed shifts each layer to a genuinely
// different region of the noise field, preventing inter-layer correlation.
// =============================================================================
static const int HARMONIC_SEED_OFFSETS[] = { 0, 7193, 14891, 22039, 31337, 41351 };
static const int MAX_HARMONICS           = 6;

// Slightly irrational lacunarity.
// Standard fBm uses 2.0 (each octave exactly doubles frequency).
// 2.17 prevents any pair of harmonics from sharing a common frequency
// multiple, which would produce faint repetition stripes over long terrain.
static const float LACUNARITY = 2.17f;


// =============================================================================
// hashGrad
// =============================================================================
float PerlinNoise::hashGrad(int latticeX, int seed) {
    // Step 1: linear combine two different primes.
    // Chosen so (latticeX=0, seed=0) doesn't trivially produce 0,
    // and so horizontal and seed dimensions have different "weights".
    int n = latticeX * 1619 + seed * 31337;

    // Step 2: XOR-shift.
    // Left-shift by 13 then XOR with original — high bits now influence
    // low bits, breaking the pure linearity from step 1.
    n = (n << 13) ^ n;

    // Step 3: Polynomial scramble.
    // n * (n*n*15731 + 789221) + 1376312589
    // Constants from Numerical Recipes — different from Ken Perlin's own
    // perm[] array, so bit patterns diverge from any textbook implementation.
    n = n * (n * n * 15731 + 789221) + 1376312589;

    // Extract gradient direction from bit 17.
    // Bit 17 is far enough from the low bits (which are most influenced by
    // the polynomial mod) to be effectively independent.
    return ((n >> 17) & 1) ? 1.0f : -1.0f;
}


// =============================================================================
// smoothStep
// =============================================================================
float PerlinNoise::smoothStep(float t) {
    // Cubic Hermite: 3t^2 - 2t^3
    // Factor form: t*t*(3 - 2*t)  — one fewer multiply than expanded.
    // This is the same smoothstep used in GPU shaders (GLSL smoothstep),
    // which gives it a recognizable name for viva explanation:
    //   "We used the standard GLSL smoothstep curve for C1 continuity."
    return t * t * (3.0f - 2.0f * t);
}


// =============================================================================
// lerp
// =============================================================================
float PerlinNoise::lerp(float a, float b, float t) {
    // Standard lerp. Written as a + t*(b-a) rather than (1-t)*a + t*b
    // to reduce float rounding error when t is close to 0.
    return a + t * (b - a);
}


// =============================================================================
// sampleRaw
// =============================================================================
float PerlinNoise::sampleRaw(float x, int seed) {
    // Integer lattice points that bracket x.
    // floorf is used (not int cast) because for negative x:
    //   (int)(-0.3f) = 0   <- WRONG, truncates toward zero
    //   floorf(-0.3f) = -1 <- CORRECT, rounds toward negative infinity
    // This matters when the player scrolls left past x=0.
    int x0 = static_cast<int>(floorf(x));
    int x1 = x0 + 1;

    // Fractional offset within [x0, x1]: always in [0, 1)
    float dx = x - static_cast<float>(x0);

    // Gradient contributions.
    // For a 1D gradient noise: contribution = gradient * (query - lattice_point)
    //   At x0: distance is +dx (x is to the right of x0)
    //   At x1: distance is (dx - 1.0f) (x is to the left of x1, so negative)
    float g0 = hashGrad(x0, seed) * dx;
    float g1 = hashGrad(x1, seed) * (dx - 1.0f);

    // Smooth blend weight using the cubic Hermite curve
    float u = smoothStep(dx);

    // Blend between the two gradient contributions.
    // Result is in approximately [-0.5, 0.5] for this single octave.
    return lerp(g0, g1, u);
}


// =============================================================================
// fractal
// =============================================================================
float PerlinNoise::fractal(float x,
                            float amplitude,
                            float frequency,
                            float persistence,
                            int   harmonics,
                            int   seed) {
    float result   = 0.0f;
    float maxValue = 0.0f;  // Tracks theoretical maximum for normalization
    float amp      = amplitude;
    float freq     = frequency;

    // Cap harmonics at MAX_HARMONICS to avoid array overrun on seed offsets.
    int h = 0;
    while (h < harmonics && h < MAX_HARMONICS) {

        // Each harmonic uses seed + HARMONIC_SEED_OFFSETS[h] so that
        // harmonic 0 and harmonic 1 sample from genuinely different
        // regions of the gradient field. Without this, higher-frequency
        // harmonics would partially correlate with the base layer,
        // making the sum look "over-reinforced" at common peaks.
        result   += sampleRaw(x * freq, seed + HARMONIC_SEED_OFFSETS[h]) * amp;
        maxValue += amp;

        // Next harmonic: higher frequency (terrain detail), lower amplitude (quieter)
        amp  *= persistence;  // Persistence < 1.0 ensures harmonics decay
        freq *= LACUNARITY;   // 2.17 instead of 2.0 — see header comment

        h++;
    }

    // Normalize the signed sum to [0.0, 1.0].
    // Dividing by maxValue accounts for variable harmonic count and
    // persistence — a profile with 5 harmonics and high persistence
    // accumulates more amplitude, so the divisor scales accordingly.
    // The * 0.5f + 0.5f maps the signed range to unsigned [0, 1].
    if (maxValue > 0.0f) {
        result = (result / maxValue) * 0.5f + 0.5f;
    }

    // Clamp to [0, 1] — floating point arithmetic can push slightly outside
    if (result < 0.0f) result = 0.0f;
    if (result > 1.0f) result = 1.0f;

    return result;
}