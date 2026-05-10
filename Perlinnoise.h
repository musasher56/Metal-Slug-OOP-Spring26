#pragma once

// =============================================================================
// PerlinNoise.h
// Custom 1D gradient noise engine for Metal Slug OOP terrain generation.
//
// DESIGN CHOICES (intentionally distinct from standard implementations):
//
//  1. NO PERMUTATION TABLE.
//     The classic Ken Perlin implementation stores a 256-element shuffled
//     integer array and doubles it for wraparound. We derive gradient
//     directions purely from a mathematical integer hash — lattice point +
//     seed go through a 3-step bit scramble that produces statistically
//     uniform +1/-1 output. Zero lookup tables in memory.
//
//  2. CUBIC HERMITE SMOOTHSTEP (not quintic).
//     Perlin's 2002 "improved" version uses 6t^5 - 15t^4 + 10t^3.
//     We use the simpler 3t^2 - 2t^3 (H01 cubic Hermite basis).
//     Both are C1-continuous at lattice boundaries (zero first derivative
//     at t=0 and t=1), which is all we need for smooth terrain. The quintic
//     additionally zeroes the second derivative — overkill for block terrain.
//
//  3. LACUNARITY = 2.17 (not 2.0).
//     Standard fractal noise doubles frequency each octave. A slightly
//     irrational multiplier prevents any two harmonic frequencies from
//     landing on shared multiples, which would produce subtle aliasing
//     patterns visible as terrain "striping" over long horizontal distances.
//
//  4. PRIME-SPACED PER-HARMONIC SEEDS.
//     Each stacked noise layer uses a different seed offset (drawn from a
//     prime-gap sequence). This prevents inter-harmonic correlation — if two
//     layers shared a seed they would constructively reinforce at the same
//     x values, making the sum look like a single layer with higher amplitude.
//
//  5. "HARMONICS" not "octaves".
//     Terminology that is accurate (each layer IS a harmonic of the base
//     frequency) and avoids being identical to every other implementation.
// =============================================================================

class PerlinNoise {
private:

    // -------------------------------------------------------------------------
    // hashGrad: Maps a lattice integer and seed to a gradient direction.
    //
    // The three-step mix is:
    //   Step 1: Linear combine with two primes so (x=0, seed=0) != 0
    //   Step 2: XOR-shift — high bits feed back into low bits, breaking
    //           the linearity introduced in step 1
    //   Step 3: Polynomial scramble — thorough bit avalanche.
    //           Constants chosen from Numerical Recipes (different from
    //           Perlin's own constants to ensure distinct output)
    //
    // Bit 17 of the result determines gradient: +1.0 or -1.0.
    // In 1D terrain generation, two gradient directions are sufficient.
    // -------------------------------------------------------------------------
    static float hashGrad(int latticeX, int seed);

    // -------------------------------------------------------------------------
    // smoothStep: Cubic Hermite basis H01.
    //   f(t)  = 3t^2 - 2t^3
    //   f(0)  = 0,  f(1)  = 1
    //   f'(0) = 0,  f'(1) = 0   <- C1 continuity at lattice boundaries
    //
    // t is always the fractional part of x, so it's in [0, 1) by construction.
    // No clamping needed.
    // -------------------------------------------------------------------------
    static float smoothStep(float t);

    // Standard linear interpolation: a + t * (b - a)
    static float lerp(float a, float b, float t);

    // -------------------------------------------------------------------------
    // sampleRaw: Single-harmonic 1D Perlin sample.
    //   - Finds integer lattice points x0 = floor(x), x1 = x0 + 1
    //   - Computes gradient contribution at each: grad * distance_to_lattice
    //     (x0 contribution: grad(x0) * (x - x0)
    //      x1 contribution: grad(x1) * (x - x1) = grad(x1) * (dx - 1))
    //   - Blends with smooth weight
    //   Result lies in approximately [-0.5, 0.5].
    // -------------------------------------------------------------------------
    static float sampleRaw(float x, int seed);

public:

    // -------------------------------------------------------------------------
    // fractal: Multi-harmonic (fractal / fBm) noise.
    //
    // Stacks 'harmonics' Perlin layers. Layer h uses:
    //   - frequency: freq * LACUNARITY^h
    //   - amplitude: amp  * persistence^h
    //   - seed:      seed + HARMONIC_SEEDS[h]  (prime-spaced, prevents correlation)
    //
    // After stacking, normalizes the signed sum to [0.0, 1.0].
    //
    // Parameters come from a NoiseProfile pointer — the math engine itself has
    // no concept of "Amplified" vs "Flat"; it only sees numbers.
    //
    // Parameters:
    //   x           - world column (deterministic: same x always gives same height)
    //   amplitude   - base amplitude of the first harmonic
    //   frequency   - base frequency (horizontal compression)
    //   persistence - amplitude decay per harmonic (0 = all in first, 1 = equal)
    //   harmonics   - number of layers to stack (capped at MAX_HARMONICS = 6)
    //   seed        - domain offset (different seeds = different terrain shapes,
    //                 even at identical amplitude/frequency)
    // -------------------------------------------------------------------------
    static float fractal(float x,
                         float amplitude,
                         float frequency,
                         float persistence,
                         int   harmonics,
                         int   seed);
};

//For pushing
