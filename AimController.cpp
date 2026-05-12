#include "AimController.h"
#include <cmath>

AimController::AimController():currentAngle(0.f),facingDirection(DIR_RIGHT){}

void AimController::update(sf::Vector2f mousePos, sf::Vector2f charCenter, int dir) {
    this->facingDirection = dir;
    float dx = mousePos.x - charCenter.x;
    float dy = mousePos.y - charCenter.y;
    if (dir == DIR_LEFT) {
        dx = -dx;
    }
    float rawAngle = atan2f(-dy, dx) * 180.f / 3.14159f;
    this->currentAngle = this->clampAngle(rawAngle);
}

float AimController::getAngle() const {
    return this->currentAngle;
}

void AimController::drawAimLine(RenderWindow& window,sf::Vector2f charCenter,float scroll,float lineLength) const
{
    float rad = this->currentAngle * 3.14159f / 180.f;
    float dirMul = (this->facingDirection == DIR_RIGHT) ? 1.f : -1.f;
    sf::Vector2f start(charCenter.x - scroll, charCenter.y);
    sf::Vector2f end(start.x + dirMul * cosf(rad) * lineLength,start.y - sinf(rad) * lineLength);

    Vertex line[2];
    line[0] = Vertex(start, Color::Red);
    line[1] = Vertex(end, Color(255, 100, 0));
    window.draw(line, 2, Lines);
    CircleShape tip(4.f);
    tip.setFillColor(Color(255, 100, 0));
    tip.setOrigin(4.f, 4.f);
    tip.setPosition(end);
    window.draw(tip);
}

float AimController::clampAngle(float raw) const {
    if (raw < -20.f)
    {return -20.f;}
    if (raw > 90.f)
    {return 90.f;}
    else
    return raw;
}
