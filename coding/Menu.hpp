#pragma once

#include "HighScore.hpp"

class Menu
{
private:
    Font font;
    unique_ptr<Text> startText;
    unique_ptr<Text> exitText;
    RectangleShape background;
    Text controlsText;
    Text rulesText;
    Text recordText;

public:
    Menu() : controlsText(font,
                          L"WASD — движение     N — сменить музыку     Esc / P — пауза", 32),
             rulesText(font, L"Собирайте монеты и избегайте метеорита.\nУ персонажа 3 жизни; бонус восстанавливает одну.\nПосле удара — 2 секунды неуязвимости, лечение временно недоступно.", 30),
             recordText(font, "", 42)
    {
        // Простой фон
        background.setSize(Vector2f(1920, 1080));
        background.setFillColor(Color(0, 0, 50, 200)); // Темно-синий полупрозрачный

        // Загрузка шрифта
        fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        startText = make_unique<Text>(font, "0", 100);
        exitText = make_unique<Text>(font, "0", 100);
        // Текст "START"
        startText->setFont(font);
        startText->setString(L"Нажмите ENTER для игры");
        startText->setCharacterSize(100);
        startText->setFillColor(Color::Green);
        startText->setPosition({350, 400});

        // Текст "EXIT"
        exitText->setFont(font);
        exitText->setString(L"Нажмите ESC для выхода");
        exitText->setCharacterSize(60);
        exitText->setFillColor(Color::White);
        exitText->setPosition({600, 550});
        controlsText.setPosition({460, 700});
        rulesText.setPosition({460, 800});
    }

    void draw(RenderWindow &window, const HighScore &record)
    {
        recordText.setString(L"Рекорд: " + to_wstring(record.get()) +
                             (record.save_failed() ? L" (не сохранён)" : L""));
        const auto bounds = recordText.getLocalBounds();
        recordText.setOrigin({bounds.position.x + bounds.size.x / 2.f, 0});
        recordText.setPosition({960, 240});
        window.draw(background);
        window.draw(recordText);
        window.draw(*startText);
        window.draw(*exitText);
        window.draw(controlsText);
        window.draw(rulesText);
    }
};
