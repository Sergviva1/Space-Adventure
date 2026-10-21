#pragma once

#include "Resources.hpp"

class GameObjects
{
protected:
    Texture textureobj;
    unique_ptr<Sprite> spriteobj;
    float speed;

public:
    GameObjects(string img, float spd) : speed(spd)
    {
        fs::path path = asset_path(img);
        require_loaded(textureobj.loadFromFile(path), path);
        spriteobj = make_unique<Sprite>(textureobj);
        respawn();
    }

    Vector2f get_position()
    {
        return spriteobj->getPosition();
    }

    FloatRect getGlobalBounds()
    {
        return spriteobj->getGlobalBounds();
    }

    virtual void respawn()
    {
        spriteobj->setPosition({2000, static_cast<float>(distrib(gen))});
    }

    virtual void move(float deltatime)
    {
        spriteobj->move({-speed * deltatime, 0});
        if (get_position().x + spriteobj->getGlobalBounds().size.x < 0)
        {
            respawn();
        }
    }

    virtual void draw(RenderWindow &window)
    {
        window.draw(*spriteobj);
    }

    virtual ~GameObjects() = default;
};

class Enemies : public GameObjects
{
public:
    Enemies() : GameObjects("meteor.png", 300)
    {
        fit_sprite(*spriteobj, {125, 125});
    }
};

class Bonus : public GameObjects
{
public:
    Bonus() : GameObjects("bonus.png", 300)
    {
        fit_sprite(*spriteobj, {92, 92});
    }
};

class Coin : public GameObjects
{
public:
    Coin() : GameObjects("coin.png", 300)
    {
        fit_sprite(*spriteobj, {96, 96});
    }
};
