#include <SFML/Graphics.hpp>
#include "Level.h"
#include "Player.h"


using namespace sf;

int screen_x = 1600;
int screen_y = 900;

int main()
{
    RenderWindow window(VideoMode(screen_x, screen_y), "Metal Slug", Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    Level  level;    
    Player player;  

    Event ev;
    while (window.isOpen())
    {
        while (window.pollEvent(ev))
        {
            if (ev.type == Event::Closed) window.close();
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            player.moveRight();
        }

        else if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            player.moveLeft();
        }
        else
        {
            player.Stop();
        }

        player.Update();

        window.clear();
        level.Draw(window);
        player.Draw(window);
        window.display();
    }

    return 0;
}