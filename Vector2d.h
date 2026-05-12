#pragma once
#include <SFML/Graphics.hpp>


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


typedef sf::Vector2f Vector2f;
