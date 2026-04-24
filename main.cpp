#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main()
{
    // Create a 1600x900 window titled "Metal Slug"
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Metal Slug");

    // Limit to 60 frames per second
    window.setFramerateLimit(60);

    // Main game loop
    while (window.isOpen())
    {
        // Event handling
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window on X button or Escape
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
        }

        // Clear screen with black
        window.clear(sf::Color::Black);

        // Draw everything here later

        // Display the frame
        window.display();
    }

    return 0;
}
