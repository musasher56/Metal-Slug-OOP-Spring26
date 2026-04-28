#pragma once
#include "Constants.h"

// WHY: AimController translates mouse/keyboard input into constrained angle (0-90°)
// Owned as a direct member of PlayerSoldier - FIX #2
class AimController {
private:
    float currentAngle;
    int facingDirection;  // DIR_LEFT or DIR_RIGHT

public:
    AimController();
    
    void update(sf::Vector2f mousePos, sf::Vector2f charPos, int dir);
    float getAngle() const;
    
private:
    float clampAngle(float raw) const;  // Clamp to 0 <= angle <= 90°
};
