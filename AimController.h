#pragma once
#include "Constants.h"

// WHY: AimController translates mouse position into a world-space angle.
class AimController {
private:
    float currentAngle;
    int   facingDirection;

public:
    AimController();

    void  update(sf::Vector2f mousePos, sf::Vector2f charCenter, int dir);

    float getAngle() const;

    void drawAimLine(RenderWindow& window,
                     sf::Vector2f charCenter,
                     float scroll,
                     float lineLength = 80.f) const;

private:
    float clampAngle(float raw) const;
};
