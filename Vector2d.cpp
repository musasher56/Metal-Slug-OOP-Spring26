#include "Vector2d.h"

Vector2d::Vector2d() : x(0.0), y(0.0) {}

Vector2d::Vector2d(double x, double y) : x(x), y(y) {}

Vector2d Vector2d::operator+(const Vector2d& other) const {
    return Vector2d(this->x + other.x, this->y + other.y);
}

Vector2d Vector2d::operator-(const Vector2d& other) const {
    return Vector2d(this->x - other.x, this->y - other.y);
}

Vector2d Vector2d::operator*(double scalar) const {
    return Vector2d(this->x * scalar, this->y * scalar);
}

bool Vector2d::operator==(const Vector2d& other) const {
    return (this->x == other.x) && (this->y == other.y);
}
