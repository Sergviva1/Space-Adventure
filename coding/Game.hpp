#pragma once

#include "Player.hpp"
#include "GameObjects.hpp"
#include "HUD.hpp"
#include "Sounds.hpp"
#include "Menu.hpp"

class Game
{
    enum class State
    {
        Menu,
        Playing,
        Paused,
        GameOver
    };

private:
    RenderWindow window;
    Clock clock;
    float deltatime;
    HighScore record;

    Player player;
    HUD hud;
    Sounds sounds;
    Enemies meteorit;
    Bonus bonus;
    Coin coin;
    Menu menu;
    PauseScreen pauseScreen;

    State state;
    bool focused = true;

    void ProcessEvents()
    {
        while (const optional event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                continue;
            }

            if (event->is<Event::FocusLost>())
            {
                focused = false;
                if (state == State::Playing)
                    pauseGame();
            }
            else if (event->is<Event::FocusGained>())
            {
                focused = true;
            }

            if (focused && window.isOpen())
            {
                if (const auto *keyPressed = event->getIf<Event::KeyPressed>())
                {
                    handleKeyPressed(keyPressed->code);
                }
            }
        }
    }

    void pauseGame()
    {
        state = State::Paused;
        sounds.pause();
    }

    void resumeGame()
    {
        clock.restart();
        sounds.resume();
        state = State::Playing;
    }

    void returnToMenu()
    {
        sounds.stop_music();
        sounds.stop_effects();
        state = State::Menu;
    }

    void handleKeyPressed(Keyboard::Key key)
    {
        if (state == State::Menu)
        {
            if (key == Keyboard::Key::Enter)
            {
                startGame();
            }
            else if (key == Keyboard::Key::Escape)
            {
                window.close();
            }
            return;
        }

        if (state == State::Playing)
        {
            if (key == Keyboard::Key::Escape || key == Keyboard::Key::P)
            {
                pauseGame();
            }
            else if (key == Keyboard::Key::N)
            {
                sounds.switch_tracks();
            }
            return;
        }

        if (state == State::Paused)
        {
            if (key == Keyboard::Key::Escape || key == Keyboard::Key::P || key == Keyboard::Key::Enter)
            {
                resumeGame();
            }
            else if (key == Keyboard::Key::M)
            {
                returnToMenu();
            }
            else if (key == Keyboard::Key::Q)
            {
                window.close();
            }
            return;
        }

        if (key == Keyboard::Key::R || key == Keyboard::Key::Enter)
        {
            startGame();
        }
        else if (key == Keyboard::Key::M)
        {
            returnToMenu();
        }
        else if (key == Keyboard::Key::Escape)
        {
            window.close();
        }
    }

    void update()
    {
        deltatime = min(clock.restart().asSeconds(), 0.05f);
        player.move(deltatime);
        player.invincibility(deltatime);
        hud.update_background(deltatime);
        meteorit.move(deltatime);
        bonus.move(deltatime);
        coin.move(deltatime);
        checkCollisions();
    }

    void checkCollisions()
    {
        if (player.getGlobalBounds().findIntersection(meteorit.getGlobalBounds()))
        {
            if (player.damage())
            {
                sounds.playDamageSound();
                hud.update_healthbar(player.get_health());
                player.respawn();
                if (player.get_health() == 0)
                {
                    state = State::GameOver;
                    sounds.stop_music();
                    sounds.playGameoverSound();
                    return;
                }
            }
        }

        if (player.getGlobalBounds().findIntersection(bonus.getGlobalBounds()))
        {
            int old_health = player.get_health();
            player.heal();
            hud.update_healthbar(player.get_health());
            if (player.get_health() > old_health)
            {
                sounds.playHealSound();
                bonus.respawn();
            }
        }

        if (player.getGlobalBounds().findIntersection(coin.getGlobalBounds()))
        {
            hud.update_scoredisplay();
            record.update(hud.get_score());
            sounds.playCoinSound();
            coin.respawn();
        }
    }

    void render()
    {
        window.clear(Color::Black);
        if (state == State::Menu)
        {
            menu.draw(window, record);
        }
        else if (state == State::Playing || state == State::Paused)
        {
            draw_game();
            if (state == State::Paused)
                pauseScreen.draw(window);
        }
        else
        {
            hud.draw(window);
            hud.draw_gameover(window, hud.get_score(), record);
        }
        window.display();
    }

    void draw_game()
    {
        hud.draw(window);
        player.draw(window);
        meteorit.draw(window);
        bonus.draw(window);
        coin.draw(window);
    }

    void startGame()
    {
        player.reset();
        hud.reset(player.get_health());
        meteorit.respawn();
        bonus.respawn();
        coin.respawn();
        clock.restart();
        sounds.stop_effects();
        sounds.restart_music();
        state = State::Playing;
    }

public:
    explicit Game(const fs::path &recordPath = high_score_path())
        : window(VideoMode::getDesktopMode(), L"Игра", sf::State::Fullscreen),
          deltatime(0), record(recordPath), state(State::Menu)
    {
        window.setKeyRepeatEnabled(false);
        window.setFramerateLimit(120);
        View view(FloatRect({0, 0}, {1920, 1080}));
        const auto size = window.getSize();
        const float ratio = (static_cast<float>(size.x) / size.y) / (1920.f / 1080.f);
        if (ratio > 1.f)
            view.setViewport(FloatRect({(1.f - 1.f / ratio) / 2.f, 0}, {1.f / ratio, 1}));
        else
            view.setViewport(FloatRect({0, (1.f - ratio) / 2.f}, {1, ratio}));
        window.setView(view);
        fs::path iconpath = asset_path("icon.png");
        Image icon;
        require_loaded(icon.loadFromFile(iconpath), iconpath);
        window.setIcon(icon);
    }

    void run()
    {
        while (window.isOpen())
        {
            ProcessEvents();
            if (window.isOpen() && state == State::Playing)
            {
                update();
            }
            if (window.isOpen())
            {
                render();
            }
        }
    }
};
