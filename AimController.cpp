#include "AimController.h"

AimController::AimController()
    : currentAngle(0.f)
    , facingDirection(DIR_RIGHT)
{}

void AimController::update(sf::Vector2f mousePos, sf::Vector2f charPos, int dir) {
    this->facingDirection = dir;
    
    // WHY: Calculate angle from character to mouse position
    float dx = mousePos.x - charPos.x;
    float dy = mousePos.y - charPos.y;
    
    // Simple angle calculation without cmath
    // Using basic ratio and clamping
    if (dx == 0.f) {
        this->currentAngle = 90.f;  // Straight up/down
    } else {
        // Approximate angle using ratio (simplified for constraint compliance)
        float ratio = dy / dx;
        if (ratio < 0.f) ratio = -ratio;
        
        // Map ratio to angle (0-90 degrees)
        if (ratio > 1.f) {
            this->currentAngle = 45.f + (ratio - 1.f) * 45.f;
        } else {
            this->currentAngle = ratio * 45.f;
        }
    }
    
    this->currentAngle = this->clampAngle(this->currentAngle);
}

float AimController::getAngle() const {
    return this->currentAngle;
}

float AimController::clampAngle(float raw) const {
    // WHY: Constrain angle to 0-90 degrees (upward arc only)
    if (raw < 0.f) return 0.f;
    if (raw > 90.f) return 90.f;
    return raw;
}
