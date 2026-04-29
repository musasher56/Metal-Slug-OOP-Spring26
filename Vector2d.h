#pragma once
#include <SFML/Graphics.hpp>

// WHY: Precision 2D vector for physics, NEAT inputs, noise sampling
// Using double for accuracy in calculations
class Vector2d {
public:
    double x;
    double y;

    Vector2d();
    Vector2d(double x, double y);

    Vector2d operator+(const Vector2d& other) const;
    Vector2d operator-(const Vector2d& other) const;
    Vector2d operator*(double scalar) const;
    bool operator==(const Vector2d& other) const;
};

// WHY: SFML Vector2f wrapper alias for entity positions (SFML uses float)
typedef sf::Vector2f Vector2f;
