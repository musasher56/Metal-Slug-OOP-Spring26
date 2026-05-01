#include "AimController.h"
#include <cmath>

AimController::AimController()
    : currentAngle(0.f)
    , facingDirection(DIR_RIGHT)
{}

// ============================================================
// update — full 360° aiming, sprite center as reference
// ============================================================
// WHAT CHANGED FROM BEFORE:
//
//   OLD: used this->position (top-left of sprite) as reference point.
//        Clamp to 0-90° per-direction.
//        Result: aim felt stuck because mouse had to be above the TOP-LEFT
//        corner of the sprite, not above the character's body.
//
//   NEW: caller passes spriteCenter (position + half width/height).
//        Full 360° — no clamp. Bullet goes wherever the mouse is.
//        atan2(-dy, dx) gives angle 0=right, 90=up, 180=left, 270=down.
//
// WHY sprite center?
//   The character is drawn with position = top-left.  The visual center
//   (where aim "feels" like it originates) is the middle of the sprite.
//   Using top-left means the mouse has to be above the top-left corner
//   to register any upward angle — that's why it felt stuck at the eyes.
//
// HOW TO CALL (in PlayerSoldier::updateAim):
//   sf::Vector2f center(
//       this->position.x + 16.f,   // half of 32px sprite width
//       this->position.y + 32.f    // half of 64px sprite height
//   );
//   this->aimController.update(mousePos, center, this->direction);
// ============================================================
void AimController::update(sf::Vector2f mousePos, sf::Vector2f charCenter, int dir) {
    this->facingDirection = dir;

    float dx = mousePos.x - charCenter.x;
    float dy = mousePos.y - charCenter.y;   // positive = mouse below center

    // atan2(-dy, dx):
    //   -dy flips Y axis (screen Y is inverted) so "above" = positive angle
    //   Result in degrees: 0=right, 90=up, 180/-180=left, -90=down
    this->currentAngle = atan2f(-dy, dx) * 180.f / 3.14159f;

    // No clamp — full 360°
    // To restore 0-90 restriction later, uncomment:
    // this->currentAngle = this->clampAngle(this->currentAngle);
}

float AimController::getAngle() const {
    return this->currentAngle;
}

// ============================================================
// drawAimLine — debug visualizer
// ============================================================
// Draws a line from the character center toward the mouse.
// Call from PlayState::renderDebug() or directly in render().
//
//   aimController.drawAimLine(window, spriteCenter, scroll, 80.f);
//
// The line length (80px) is just for visibility — doesn't affect gameplay.
// ============================================================
void AimController::drawAimLine(RenderWindow& window,
                                  sf::Vector2f charCenter,
                                  float scroll,
                                  float lineLength) const
{
    float rad = this->currentAngle * 3.14159f / 180.f;

    // Start: character center in screen space
    sf::Vector2f start(charCenter.x - scroll, charCenter.y);

    // End: lineLength pixels in the aim direction
    sf::Vector2f end(
        start.x + cosf(rad) * lineLength,
        start.y - sinf(rad) * lineLength   // negate: screen Y is inverted
    );

    // Draw as two vertices (a line primitive)
    Vertex line[2];
    line[0] = Vertex(start, Color::Red);
    line[1] = Vertex(end,   Color(255, 100, 0));  // orange tip

    window.draw(line, 2, Lines);

    // Draw a small circle at the tip so it's easy to see at steep angles
    CircleShape tip(4.f);
    tip.setFillColor(Color(255, 100, 0));
    tip.setOrigin(4.f, 4.f);
    tip.setPosition(end);
    window.draw(tip);
}

float AimController::clampAngle(float raw) const {
    if (raw < 0.f)  return 0.f;
    if (raw > 90.f) return 90.f;
    return raw;
}