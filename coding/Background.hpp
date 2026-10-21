#pragma once

#include "Resources.hpp"

class Background
{
private:
    Texture texturespace;
    RectangleShape backgroundgame;
    RectangleShape backgroundgame2;
    float backgroundspeed = 100.f;
    Vector2f pos;

public:
    Background()
    {
        fs::path path = asset_path("space.png");
        require_loaded(texturespace.loadFromFile(path), path);
        backgroundgame.setTexture(&texturespace);
        backgroundgame2.setTexture(&texturespace);
        backgroundgame.setSize(Vector2f{1920, 1080});
        backgroundgame2.setSize(Vector2f{1920, 1080});
        backgroundgame2.setPosition(Vector2f{1920, 0});
    }

    void move(float deltatime)
    {
        backgroundgame.move({-backgroundspeed * deltatime, 0});
        pos = backgroundgame.getPosition();
        if (pos.x <= -1920)
            backgroundgame.move({3840, 0});
        backgroundgame2.move({-backgroundspeed * deltatime, 0});
        pos = backgroundgame2.getPosition();
        if (pos.x <= -1920)
            backgroundgame2.move({3840, 0});
    }

    void reset()
    {
        backgroundgame.setPosition({0, 0});
        backgroundgame2.setPosition({1920, 0});
    }

    void draw(RenderWindow &window)
    {
        window.draw(backgroundgame2);
        window.draw(backgroundgame);
    }
};
