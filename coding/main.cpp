#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <filesystem>
#include <fstream>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;
using namespace sf;
namespace fs = std::filesystem;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(197, 955);

// Папка с ресурсами рядом с исполняемым файлом
fs::path g_asset_dir;

// Полный путь к файлу в assets/
fs::path asset_path(const fs::path &filename)
{
    return g_asset_dir / filename;
}

// Проверка загрузки ресурса: при ошибке сообщаем путь и прекращаем запуск
void require_loaded(bool loaded, const fs::path &path)
{
    if (!loaded)
    {
        throw runtime_error("Не удалось загрузить ресурс: " + path.u8string());
    }
}

// Каталог исполняемого файла, независимо от текущей рабочей папки
fs::path executable_dir()
{
    wstring executable_path(32768, L'\0');
    const DWORD path_length = GetModuleFileNameW(
        nullptr, executable_path.data(), static_cast<DWORD>(executable_path.size()));

    if (path_length == 0 || path_length >= executable_path.size())
    {
        throw runtime_error("Не удалось определить каталог исполняемого файла.");
    }

    executable_path.resize(path_length);
    return fs::path(executable_path).parent_path();
}

// Рекорд сохранён в профиле, независимо от папки установки игры.
fs::path high_score_path()
{
    wstring directory(32768, L'\0');
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", directory.data(),
                                                 static_cast<DWORD>(directory.size()));
    if (length == 0 || length >= directory.size())
        return {};
    directory.resize(length);
    return fs::path(directory) / "SpaceAdventure" / "highscore.txt";
}

class HighScore
{
private:
    fs::path path;
    int best = 0;
    bool saveFailed = false;

    static int read(const fs::path &file)
    {
        error_code error;
        const auto size = fs::file_size(file, error);
        if (error || size > 32)
            return 0;
        ifstream input(file);
        long long value = 0;
        if (!(input >> value) || value < 0 || value > numeric_limits<int>::max())
            return 0;
        input >> ws;
        return input.eof() ? static_cast<int>(value) : 0;
    }

    bool save()
    {
        if (path.empty())
            return false;
        error_code error;
        if (!path.parent_path().empty())
            fs::create_directories(path.parent_path(), error);
        if (error)
            return false;
        // Временный файл не заменил прежний рекорд до успешного окончания записи.
        fs::path temporary = path;
        temporary += "." + to_string(GetCurrentProcessId()) + ".tmp";
        bool written = false;
        {
            ofstream output(temporary, ios::trunc);
            output << best << '\n';
            output.close();
            written = static_cast<bool>(output);
        }
        if (written && MoveFileExW(temporary.c_str(), path.c_str(),
                                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            return true;
        fs::remove(temporary, error);
        return false;
    }

public:
    explicit HighScore(const fs::path &file) : path(file), best(file.empty() ? 0 : read(file)) {}

    int get() const { return best; }
    bool save_failed() const { return saveFailed; }

    void update(int score)
    {
        if (score <= best && !saveFailed)
            return;
        best = max({best, score, path.empty() ? 0 : read(path)});
        saveFailed = !save();
        if (saveFailed)
            cerr << "Не удалось сохранить рекорд: " << path.u8string() << endl;
    }
};

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
        sprite.setPosition({clamp(position.x, -25.f, 1780.f),
                            clamp(position.y, 197.f, 955.f)});
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

// Размер спрайта ограничен игровой областью, а не разрешением исходной картинки.
void fit_sprite(Sprite& sprite, Vector2f maximum)
{
    const Vector2f size = sprite.getLocalBounds().size;
    const float scale = min(maximum.x / size.x, maximum.y / size.y);
    sprite.setScale({scale, scale});
}

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
        Sprite_HealthBar->setPosition({28, 20});
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
        score->setPosition({30, 90});
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

class Sounds
{
private:
    Music music1;
    Music music2;
    Music music3;
    int current_track;

    SoundBuffer damagebuffer;
    Sound damagesound;
    SoundBuffer coinbuffer;
    Sound coinsound;
    SoundBuffer healbuffer;
    Sound healsound;
    SoundBuffer gameoverbuffer;
    Sound gameoversound;

public:
    Sounds() : current_track(1), damagesound(damagebuffer), coinsound(coinbuffer), healsound(healbuffer), gameoversound(gameoverbuffer)
    {
        fs::path path1 = asset_path("retro-waves-of-love_92802.ogg");
        require_loaded(music1.openFromFile(path1), path1);
        fs::path path2 = asset_path("retro-dreamscape_92772.ogg");
        require_loaded(music2.openFromFile(path2), path2);
        fs::path path3 = asset_path("pixelated-adventures_92797.ogg");
        require_loaded(music3.openFromFile(path3), path3);

        fs::path damagepath = asset_path("hitHurt.ogg");
        require_loaded(damagebuffer.loadFromFile(damagepath), damagepath);
        damagesound.setBuffer(damagebuffer);
        fs::path coinpath = asset_path("pickupCoin.ogg");
        require_loaded(coinbuffer.loadFromFile(coinpath), coinpath);
        coinsound.setBuffer(coinbuffer);
        fs::path healpath = asset_path("powerUp.ogg");
        require_loaded(healbuffer.loadFromFile(healpath), healpath);
        healsound.setBuffer(healbuffer);
        fs::path gameoverpath = asset_path("explosion.ogg");
        require_loaded(gameoverbuffer.loadFromFile(gameoverpath), gameoverpath);
        gameoversound.setBuffer(gameoverbuffer);

        damagesound.setVolume(5);
        coinsound.setVolume(10);
        healsound.setVolume(5);
        gameoversound.setVolume(5);
        music1.setVolume(10);
        music2.setVolume(10);
        music3.setVolume(10);
        music1.setLooping(true);
        music2.setLooping(true);
        music3.setLooping(true);
    }

    void play1()
    {
        music2.stop();
        music3.stop();
        music1.play();
        current_track = 1;
    }

    void stop_music()
    {
        music1.stop();
        music2.stop();
        music3.stop();
    }

    void restart_music()
    {
        stop_music();
        play1();
    }

    void play2()
    {
        music1.stop();
        music3.stop();
        music2.play();
        current_track = 2;
    }

    void play3()
    {
        music1.stop();
        music2.stop();
        music3.play();
        current_track = 3;
    }

    void switch_tracks()
    {
        if (current_track == 1)
        {
            play2();
        }
        else if (current_track == 2)
        {
            play3();
        }
        else
        {
            play1();
        }
    }

    void pause()
    {
        music1.pause();
        music2.pause();
        music3.pause();
        damagesound.pause();
        coinsound.pause();
        healsound.pause();
        gameoversound.pause();
    }

    void resume()
    {
        if (current_track == 1)
            music1.play();
        else if (current_track == 2)
            music2.play();
        else
            music3.play();
        if (damagesound.getStatus() == SoundSource::Status::Paused)
            damagesound.play();
        if (coinsound.getStatus() == SoundSource::Status::Paused)
            coinsound.play();
        if (healsound.getStatus() == SoundSource::Status::Paused)
            healsound.play();
    }

    void stop_effects()
    {
        damagesound.stop();
        coinsound.stop();
        healsound.stop();
        gameoversound.stop();
    }

    void playDamageSound()
    {
        damagesound.play();
    }

    void playCoinSound()
    {
        coinsound.play();
    }

    void playHealSound()
    {
        healsound.play();
    }

    void playGameoverSound()
    {
        gameoversound.play();
    }
};

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

int main()
{
    try
    {
        g_asset_dir = executable_dir() / "assets";
        Game game;
        game.run();
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
        const wstring message = sf::String::fromUtf8(e.what(), e.what() + char_traits<char>::length(e.what())).toWideString();
        MessageBoxW(nullptr, message.c_str(), L"Ошибка запуска игры", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
