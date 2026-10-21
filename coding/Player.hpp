#pragma once

#include "Resources.hpp"

class Player
{
private:
    Texture texture;
    Sprite sprite;
    int health = 3;
    bool can_be_hit = true;
    bool visible = true;
    float visible_timer = 0.0f;
    float invincibility_timer = 0.0f;
    float speed = 1000.f;
    int current_frame = 0;
    float frametime = 0.f;
    float switch_time = 0.2f;
    int frame_width = 166;
    int frame_height = 130;

    void player_lock()
    {
        const Vector2f position = sprite.getPosition();
        sprite.setPosition({clamp(position.x, 5.f, 1750.f),
                            clamp(position.y, 150.f, 880.f)});
    }

public:
    Player() : sprite(texture)
    {
        fs::path path = asset_path("spritelist.png");
        require_loaded(texture.loadFromFile(path), path);
        sprite.setTexture(texture);
        sprite.setTextureRect(IntRect({0, 0}, {166, 130}));
        sprite.setPosition({150.f, 550.f});
    }

    FloatRect getGlobalBounds()
    {
        return sprite.getGlobalBounds();
    }

    int get_health()
    {
        return health;
    }

    bool damage()
    {
        if (!can_be_hit)
        {
            return false;
        }

        --health;
        can_be_hit = false;
        return true;
    }

    void heal()
    {
        if (can_be_hit == true && health < 3)
        {
            health = health + 1;
        }
    }

    void invincibility(float deltatime)
    {
        if (can_be_hit == false)
        {
            invincibility_timer += deltatime;
            visible_timer += deltatime;

            if (visible_timer >= 0.2)
            {
                visible = !visible;
                visible_timer = 0.0;
            }

            if (invincibility_timer >= 2.0)
            {
                can_be_hit = true;
                invincibility_timer = 0.0f;
                visible = true;
            }
        }
    }

    void respawn()
    {
        sprite.setPosition({150.f, 550.f});
    }

    void reset()
    {
        health = 3;
        can_be_hit = true;
        visible = true;
        visible_timer = 0.0f;
        invincibility_timer = 0.0f;
        current_frame = 0;
        frametime = 0.0f;
        sprite.setTextureRect(IntRect({0, 0}, {frame_width, frame_height}));
        respawn();
    }

    void move(float deltatime)
    {
        Vector2f direction{
            static_cast<float>(Keyboard::isKeyPressed(Keyboard::Key::D)) -
                static_cast<float>(Keyboard::isKeyPressed(Keyboard::Key::A)),
            static_cast<float>(Keyboard::isKeyPressed(Keyboard::Key::S)) -
                static_cast<float>(Keyboard::isKeyPressed(Keyboard::Key::W))};
        const float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.f)
            sprite.move(direction / length * speed * deltatime);

        player_lock();

        frametime = frametime + deltatime;
        if (frametime >= switch_time)
        {
            frametime = 0.f;
            current_frame = (current_frame + 1) % 2;
            int y0ffset = current_frame * frame_height;
            sprite.setTextureRect(IntRect({0, y0ffset}, {frame_width, frame_height}));
        }
    }

    void draw(RenderWindow &window)
    {
        if (visible == true)
        {
            window.draw(sprite);
        }
    }
};
