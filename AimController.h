#pragma once
#include "Constants.h"

// WHY: AimController translates mouse position into a world-space angle.
// Full 360° — no directional clamp.
// Owned as a direct member of PlayerSoldier.
class AimController {
private:
    float currentAngle;       // degrees: 0=right, 90=up, 180=left, -90=down
    int   facingDirection;    // DIR_LEFT or DIR_RIGHT

public:
    AimController();

    // charCenter = position + half sprite size (NOT top-left corner)
    // WHY center? Using top-left causes aim to feel stuck near the eyes.
    void  update(sf::Vector2f mousePos, sf::Vector2f charCenter, int dir);

    float getAngle() const;

    // Debug visualizer — draws a red line from character toward mouse
    // Call from renderDebug() to see aim direction in real time
    void drawAimLine(RenderWindow& window,
                     sf::Vector2f charCenter,
                     float scroll,
                     float lineLength = 80.f) const;

private:
    float clampAngle(float raw) const;  // kept for optional 0-90 restriction
};