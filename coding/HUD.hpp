#pragma once

#include "Background.hpp"
#include "HighScore.hpp"

class GamePanel
{
private:
    Texture gamepaneltexture;
    RectangleShape GamePanelshape;

public:
    GamePanel()
    {
        fs::path path = asset_path("gamepanel.png");
        require_loaded(gamepaneltexture.loadFromFile(path), path);
        GamePanelshape.setTexture(&gamepaneltexture);
        GamePanelshape.setSize(Vector2f{1920, 1080});
        GamePanelshape.setPosition(Vector2f{0, 0});
    }

    void draw(RenderWindow &window)
    {
        window.draw(GamePanelshape);
    }
};

class GameOver
{
private:
    Texture GameOver_Texture;
    unique_ptr<Sprite> GameOver_Sprite;
    Font font;
    unique_ptr<Text> resultText;
    unique_ptr<Text> controlsText;
    Text recordText;

public:
    GameOver() : recordText(font, "", 36)
    {
        fs::path path = asset_path("gameover.png");
        require_loaded(GameOver_Texture.loadFromFile(path), path);
        path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);

        GameOver_Sprite = make_unique<Sprite>(GameOver_Texture);
        fit_sprite(*GameOver_Sprite, {1000, 340});
        const auto bounds = GameOver_Sprite->getLocalBounds();
        GameOver_Sprite->setOrigin(bounds.position + bounds.size / 2.f);
        GameOver_Sprite->setPosition({960, 480});
        resultText = make_unique<Text>(font, "", 56);
        resultText->setFillColor(Color::White);
        controlsText = make_unique<Text>(font, L"R / Enter — новая игра     M — меню     Esc — выход", 36);
        controlsText->setFillColor(Color::White);

        const FloatRect plaqueBounds = GameOver_Sprite->getGlobalBounds();
        const float plaqueCenterX = plaqueBounds.position.x + plaqueBounds.size.x / 2.f;
        resultText->setPosition({plaqueCenterX, 700});
        const FloatRect controlsBounds = controlsText->getLocalBounds();
        controlsText->setOrigin({controlsBounds.position.x + controlsBounds.size.x / 2.f, 0.f});
        controlsText->setPosition({plaqueCenterX, 870});
    }

    void draw(RenderWindow &window, int finalScore, const HighScore &record)
    {
        window.draw(*GameOver_Sprite);
        resultText->setString(L"Ваш счёт: " + to_wstring(finalScore));
        const FloatRect resultBounds = resultText->getLocalBounds();
        resultText->setOrigin({resultBounds.position.x + resultBounds.size.x / 2.f, 0.f});
        const FloatRect plaqueBounds = GameOver_Sprite->getGlobalBounds();
        resultText->setPosition({plaqueBounds.position.x + plaqueBounds.size.x / 2.f, 700});
        recordText.setString(L"Рекорд: " + to_wstring(record.get()) +
                             (record.save_failed() ? L" (не сохранён)" : L""));
        const auto recordBounds = recordText.getLocalBounds();
        recordText.setOrigin({recordBounds.position.x + recordBounds.size.x / 2.f, 0});
        recordText.setPosition({plaqueBounds.position.x + plaqueBounds.size.x / 2.f, 785});
        window.draw(*resultText);
        window.draw(recordText);
        window.draw(*controlsText);
    }
};

class PauseScreen
{
private:
    Font font;
    Text title;
    Text controls;
    RectangleShape overlay;

public:
    PauseScreen() : title(font, L"Пауза", 80),
                    controls(font, L"Esc / P / Enter — продолжить\nM — главное меню\nQ — выход из игры\n\nWASD — движение     N — сменить музыку", 36)
    {
        const fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        overlay.setSize({1920, 1080});
        overlay.setFillColor(Color(0, 0, 30, 210));
        title.setPosition({800, 330});
        controls.setPosition({480, 480});
    }

    void draw(RenderWindow &window)
    {
        window.draw(overlay);
        window.draw(title);
        window.draw(controls);
    }
};

class HealthBar
{
private:
    Texture Texture_HealthBar;
    unique_ptr<Sprite> Sprite_HealthBar;
    unique_ptr<Sprite> emptyBar;

public:
    HealthBar()
    {
        fs::path path = asset_path("healthbar.png");
        require_loaded(Texture_HealthBar.loadFromFile(path), path);
        Sprite_HealthBar = make_unique<Sprite>(Texture_HealthBar);
        Sprite_HealthBar->setPosition({200, 15});
        Sprite_HealthBar->setScale({0.65f, 0.65f});
        emptyBar = make_unique<Sprite>(Texture_HealthBar);
        emptyBar->setTextureRect(IntRect({0, 207}, {312, 104}));
        emptyBar->setPosition(Sprite_HealthBar->getPosition());
        emptyBar->setScale(Sprite_HealthBar->getScale());
        update(3);
    }

    void update(int health)
    {
        const int lives = clamp(health, 0, 3);
        Sprite_HealthBar->setColor(lives > 0 ? Color::White : Color::Transparent);
        // Полная полоска обрезана по числу жизней поверх пустой, включая 1/3.
        const int width = lives == 3 ? 312 : 76 + 205 * lives / 3;
        Sprite_HealthBar->setTextureRect(IntRect({0, 0}, {width, 104}));
    }

    void draw(RenderWindow &window)
    {
        window.draw(*emptyBar);
        window.draw(*Sprite_HealthBar);
    }
};

class ScoreDisplay
{
private:
    Font font;
    unique_ptr<Text> score;
    int score_value = 0;

public:
    ScoreDisplay()
    {
        fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        score = make_unique<Text>(font, "0", 100);
        score->setPosition({420, 15});
        score->setScale({0.55f, 0.55f});
        score->setFillColor(Color::White);
        score->setOutlineColor(Color(5, 5, 30));
        score->setOutlineThickness(3.f);
    }

    int get_score() const
    {
        return score_value;
    }

    void reset()
    {
        score_value = 0;
        score->setString("0");
    }

    void score_plus()
    {
        if (score_value < numeric_limits<int>::max())
            ++score_value;
        score->setString(to_string(score_value));
    }

    void draw(RenderWindow &window)
    {
        window.draw(*score);
    }
};

class HUD
{
private:
    Background background;
    GamePanel gamepanel;
    GameOver gameover;
    HealthBar healthbar;
    ScoreDisplay score;

public:
    HUD() : background(), gamepanel(), gameover(), healthbar(), score() {}

    void update_background(float deltatime)
    {
        background.move(deltatime);
    }

    void update_healthbar(int health)
    {
        healthbar.update(health);
    }

    void update_scoredisplay()
    {
        score.score_plus();
    }

    void draw_gameover(RenderWindow &window, int finalScore, const HighScore &record)
    {
        gameover.draw(window, finalScore, record);
    }

    void draw(RenderWindow &window)
    {
        background.draw(window);
        gamepanel.draw(window);
        healthbar.draw(window);
        score.draw(window);
    }

    void reset(int health)
    {
        background.reset();
        healthbar.update(health);
        score.reset();
    }

    int get_score() const
    {
        return score.get_score();
    }
};
