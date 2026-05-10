#include "NoiseProfile.h"

// =============================================================================
// NoiseProfile::create() — Factory implementation.
//
// P5 COMPLIANCE: No switch statement anywhere in this file.
//
// WHY FUNCTION POINTER ARRAY INSTEAD OF SWITCH:
//   A switch statement bakes the branching logic into the factory body.
//   Every time you add a new profile type you must open this file, add a
//   new case, and recompile. The dispatch loop is forever tangled with the
//   list of concrete types.
//
//   A function pointer array treats each constructor as DATA. The dispatch
//   loop is trivially: "index into array, call what you find."
//   Adding a 4th profile = append one lambda. The loop never changes.
//   This is the Open/Closed Principle applied at the language level.
//
// VTABLE CONNECTION:
//   Each lambda returns a NoiseProfile* pointing to a heap-allocated
//   derived object. The pointer's vtable slot points to the concrete
//   class's virtual function table. When Level later calls
//   profile->getAmplitude(), the call goes through the vtable to the
//   correct derived implementation — zero type-checking required anywhere.
//
// INDEX MAPPING (matches Constants.h):
//   NOISE_AMPLIFIED = 0  →  profileCreators[0]  →  AmplifiedProfile
//   NOISE_FLAT      = 1  →  profileCreators[1]  →  FlatProfile
//   NOISE_NORMAL    = 2  →  profileCreators[2]  →  NormalProfile
// =============================================================================

// Function pointer type: a function that takes no args and returns NoiseProfile*
typedef NoiseProfile* (*ProfileCreator)();

// Static array of creator lambdas.
// 'static' limits linkage to this translation unit — no symbol collision risk.
// Each lambda captures nothing (no closure state), so they decay cleanly to
// raw function pointers (C++11 guarantee for captureless lambdas).
static ProfileCreator profileCreators[] = {

    // [0] NOISE_AMPLIFIED — dramatic mountains and deep ocean
    []() -> NoiseProfile* { return new AmplifiedProfile(); },

    // [1] NOISE_FLAT — gentle plains, shallow sea
    []() -> NoiseProfile* { return new FlatProfile(); },

    // [2] NOISE_NORMAL — balanced, uniformly distributed terrain
    []() -> NoiseProfile* { return new NormalProfile(); },
};

// Profile count derived from array size rather than a hardcoded magic number.
// If you add a 4th profile to the array above, this automatically reflects it.
static const int PROFILE_COUNT =
    static_cast<int>(sizeof(profileCreators) / sizeof(profileCreators[0]));


NoiseProfile* NoiseProfile::create(int type) {
    // Bounds check before indexing — never dereference an invalid pointer.
    if (type >= 0 && type < PROFILE_COUNT) {
        // Call the creator function at this index.
        // The returned pointer's vtable is already wired to the correct
        // concrete class — the caller needs no type knowledge whatsoever.
        return profileCreators[type]();
    }

    // Out-of-range type: silently default to Normal.
    // Failing loudly here would be valid too, but in a game context a
    // graceful fallback is preferable to a crash during demo.
    return new NormalProfile();
}