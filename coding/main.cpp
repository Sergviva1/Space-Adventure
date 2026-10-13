#define SFML_STATIC
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <random>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;
using namespace sf;
namespace fs = std::filesystem;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(197,955);

// Папка с ресурсами рядом с исполняемым файлом
fs::path g_asset_dir;

// Полный путь к файлу в assets/
fs::path asset_path(const fs::path &filename) {
    return g_asset_dir / filename;
}

// Проверка загрузки ресурса: при ошибке сообщаем путь и прекращаем запуск
void require_loaded(bool loaded, const fs::path &path) {
    if (!loaded) {
        throw runtime_error("Не удалось загрузить ресурс: " + path.u8string());
    }
}

// Каталог исполняемого файла, независимо от текущей рабочей папки
fs::path executable_dir() {
    wstring executable_path(32768, L'\0');
    const DWORD path_length = GetModuleFileNameW(
        nullptr, executable_path.data(), static_cast<DWORD>(executable_path.size()));

    if (path_length == 0 || path_length >= executable_path.size()) {
        throw runtime_error("Не удалось определить каталог исполняемого файла.");
    }

    executable_path.resize(path_length);
    return fs::path(executable_path).parent_path();
}

class Player{
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

    void player_lock(){
        Vector2f posplayer = sprite.getPosition();
        if (posplayer.x > 1780) {
            sprite.setPosition({1780, posplayer.y});
        }
        if (posplayer.x < -25) {
            sprite.setPosition({-25, posplayer.y});
        }
        if (posplayer.y > 955) {
             sprite.setPosition({posplayer.x, 955});
        }
        if (posplayer.y < 197) {
            sprite.setPosition({posplayer.x, 197});
        }
    }

public:
    Player() : sprite(texture) {
        fs::path path = asset_path("spritelist.png");
        require_loaded(texture.loadFromFile(path), path);
        sprite.setTexture(texture);
        sprite.setTextureRect(IntRect({0,0}, {166, 130}));
        sprite.setPosition({150.f, 550.f});
    }

    FloatRect getGlobalBounds() {
        return sprite.getGlobalBounds();
    }

    int get_health() {
        return health;
    }

    bool damage() {
        if (!can_be_hit) {
            return false;
        }

        --health;
        can_be_hit = false;
        return true;
    }

    void heal() {
        if (can_be_hit == true && health < 3) {
            health = health + 1;
        }
    }
    
    void invincibility(float deltatime) {
        if (can_be_hit == false) {
            invincibility_timer += deltatime;
            visible_timer += deltatime;

            if (visible_timer >= 0.2) {
                visible = !visible;
                visible_timer = 0.0;
            }

            if (invincibility_timer >= 2.0) {
                can_be_hit = true;
                invincibility_timer = 0.0f;
                visible = true;
            }
        }
    }

    void respawn() {
        sprite.setPosition({150.f, 550.f});
    }

    void reset() {
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
    
    void move(float deltatime){
        if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
        sprite.move({-speed * deltatime, 0.f});
        }

        if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
        sprite.move({speed * deltatime, 0.f});
        }

        if (Keyboard::isKeyPressed(Keyboard::Key::W)) {
        sprite.move({0.f, -speed * deltatime});
        }
        
        if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
        sprite.move({0.f, speed * deltatime});
        }

        player_lock();



        frametime = frametime + deltatime;
        if (frametime >= switch_time) {
            frametime = 0.f;
            current_frame = (current_frame + 1) % 2;
            int y0ffset = current_frame * frame_height;
            sprite.setTextureRect(IntRect({0, y0ffset}, {frame_width, frame_height}));
        } 
    }


    void draw(RenderWindow &window){
        if (visible == true) {
            window.draw(sprite);
        }
    }

};

class GameObjects{
    protected:
    Texture textureobj;
    Sprite *spriteobj;
    float speed;
public:
    GameObjects(string img, float spd) : speed(spd){
        fs::path path = asset_path(img);
        require_loaded(textureobj.loadFromFile(path), path);
        spriteobj = new Sprite(textureobj);
        respawn();
    }

    Vector2f get_position(){
        return spriteobj->getPosition();
    }

    FloatRect getGlobalBounds(){
        return spriteobj->getGlobalBounds();
    }

    virtual void respawn(){
    spriteobj->setPosition({2000, static_cast<float>(distrib(gen))});   
    }

    virtual void move (float deltatime) {
        spriteobj->move({-speed * deltatime, 0});
        if (get_position().x < -100) {
            respawn();
        }
    }
    
    virtual void draw(RenderWindow &window) {
        window.draw(*spriteobj);
    }
    
    ~GameObjects(){
        delete spriteobj;
    }
};

class Enemies : public GameObjects {
public:
    Enemies() : GameObjects("meteor.png", 300) {
        spriteobj->setScale({0.15f, 0.15f});
    }
};

class Bonus : public GameObjects{
public:
    Bonus() : GameObjects("bonus.png",300){
    }
};

class Coin : public GameObjects {
public:
    Coin() : GameObjects("coin.png",300){
        spriteobj->setScale({0.45f,0.45f});
    }
};

class Background {
    private:
    Texture texturespace;
    RectangleShape backgroundgame;
    RectangleShape backgroundgame2;
    float backgroundspeed = 100.f;
    Vector2f pos;
    public:
    Background(){
        fs::path path = asset_path("space.jpg");
        require_loaded(texturespace.loadFromFile(path), path);
        backgroundgame.setTexture(&texturespace);
        backgroundgame2.setTexture(&texturespace);
        backgroundgame.setSize(Vector2f{1920,1080});
        backgroundgame2.setSize(Vector2f{1920,1080});
        backgroundgame2.setPosition(Vector2f {1920,0});
    }
    
    void move(float deltatime){
        backgroundgame.move({-backgroundspeed * deltatime,0});
        pos = backgroundgame.getPosition();
        if (pos.x<-1920) backgroundgame.setPosition({1920,pos.y});
        backgroundgame2.move({-backgroundspeed * deltatime,0});
        pos = backgroundgame2.getPosition();
        if (pos.x<-1920) backgroundgame2.setPosition({1920,pos.y});
    }

    void reset() {
        backgroundgame.setPosition({0, 0});
        backgroundgame2.setPosition({1920, 0});
    }
    
    void draw(RenderWindow &window) {
        window.draw(backgroundgame2);
        window.draw(backgroundgame);
    }
};

class GamePanel {
    private:
    Texture gamepaneltexture;
    RectangleShape GamePanelshape;
    public:
    GamePanel() {
        fs::path path = asset_path("gamepanel.png");
        require_loaded(gamepaneltexture.loadFromFile(path), path);
        GamePanelshape.setTexture(&gamepaneltexture);
        GamePanelshape.setSize(Vector2f{1920,720});
        GamePanelshape.setPosition(Vector2f{0,0});
    }
    
    void draw(RenderWindow &window){
        window.draw(GamePanelshape);
    }
};

class GameOver {
private:
    Texture GameOver_Texture;
    Sprite *GameOver_Sprite = nullptr;
    Font font;
    Text *resultText = nullptr;
    Text *controlsText = nullptr;
public:
    GameOver() {
        fs::path path = asset_path("gameover.png");
        require_loaded(GameOver_Texture.loadFromFile(path), path);
        path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);

        GameOver_Sprite = new Sprite (GameOver_Texture);
        GameOver_Sprite->setScale({0.4f,0.4f});
        GameOver_Sprite->setPosition({650,339});
        resultText = new Text(font, "", 56);
        resultText->setFillColor(Color::White);
        controlsText = new Text(font, L"R — новая игра     M — меню     Esc — выход", 36);
        controlsText->setFillColor(Color::White);

        const FloatRect plaqueBounds = GameOver_Sprite->getGlobalBounds();
        const float plaqueCenterX = plaqueBounds.position.x + plaqueBounds.size.x / 2.f;
        resultText->setPosition({plaqueCenterX, 700});
        const FloatRect controlsBounds = controlsText->getLocalBounds();
        controlsText->setOrigin({controlsBounds.position.x + controlsBounds.size.x / 2.f, 0.f});
        controlsText->setPosition({plaqueCenterX, 820});
    }

    void draw(RenderWindow &window, int finalScore){
        window.draw(*GameOver_Sprite);
        resultText->setString(L"Ваш счёт: " + to_wstring(finalScore));
        const FloatRect resultBounds = resultText->getLocalBounds();
        resultText->setOrigin({resultBounds.position.x + resultBounds.size.x / 2.f, 0.f});
        const FloatRect plaqueBounds = GameOver_Sprite->getGlobalBounds();
        resultText->setPosition({plaqueBounds.position.x + plaqueBounds.size.x / 2.f, 700});
        window.draw(*resultText);
        window.draw(*controlsText);
    }

    ~GameOver() {
        delete GameOver_Sprite;
        delete resultText;
        delete controlsText;
    }
};

class PauseScreen {
private:
    Font font;
    Text title;
    Text controls;
    RectangleShape overlay;
public:
    PauseScreen() : title(font, L"Пауза", 80),
        controls(font, L"Esc / P / Enter — продолжить\nM — главное меню\nQ — выход из игры\n\nWASD — движение     N — сменить музыку", 36) {
        const fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        overlay.setSize({1920, 1080});
        overlay.setFillColor(Color(0, 0, 30, 210));
        title.setPosition({800, 330});
        controls.setPosition({480, 480});
    }

    void draw(RenderWindow &window) {
        window.draw(overlay);
        window.draw(title);
        window.draw(controls);
    }
};

class HealthBar {
private:
    Texture Texture_HealthBar;
    Sprite *Sprite_HealthBar;
public:
    HealthBar() {
        fs::path path = asset_path("healthbar.png");
        require_loaded(Texture_HealthBar.loadFromFile(path), path);
        Sprite_HealthBar = new Sprite(Texture_HealthBar);
        Sprite_HealthBar->setPosition({0,0});
        Sprite_HealthBar->setScale({0.5f,0.5f});
        update(3);

    }

    void update(int health) {
        Sprite_HealthBar->setColor(Color::White);
        if (health == 3) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 0}, {312, 104}));
        } else if (health == 2) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 107}, {312, 104}));
        } else if (health == 1) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 207}, {312, 104}));
        } else {
            Sprite_HealthBar->setColor(Color(255, 255, 255, 0));
        }
    }

    void draw (RenderWindow &window) {
        window.draw(*Sprite_HealthBar);
    }
    
    ~HealthBar() {
        delete Sprite_HealthBar;
    }
};

class ScoreDisplay {
private:
    Font font;
    Text *score;
    float score_value = 0.0f;
public:
    ScoreDisplay() {
        fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        score = new Text(font, "0", 100);
        score->setPosition({3,50});
        score->setScale({0.7f,0.7f});
        score->setFillColor(Color::Black);
    }

    int get_score() const {
        return static_cast<int>(score_value);
    }

    void reset() {
        score_value = 0.0f;
        score->setString("0");
    }

    void score_plus(){
        score_value += 1;
        score->setString(to_string((int)score_value));
    }

    void draw(RenderWindow &window) {
        window.draw(*score);
    }

    ~ScoreDisplay(){
        delete score;
    }
};

class HUD {
    private:
    Background background;
    GamePanel gamepanel;
    GameOver gameover;
    HealthBar healthbar;
    ScoreDisplay score;   
public:
    HUD() : background(), gamepanel(), gameover(), healthbar(), score() {}

    void update_background(float deltatime) {
        background.move(deltatime);
    }

    void update_healthbar(int health) {
        healthbar.update(health);
    }

    void update_scoredisplay(){
        score.score_plus();
    }

    void draw_gameover(RenderWindow &window, int finalScore){
        gameover.draw(window, finalScore);
    }

    void draw(RenderWindow &window){
        background.draw(window);
        gamepanel.draw(window);
        healthbar.draw(window);
        score.draw(window);
    }

    void reset(int health) {
        background.reset();
        healthbar.update(health);
        score.reset();
    }

    int get_score() const {
        return score.get_score();
    }
};

class Sounds {
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
    Sounds() : current_track(1), damagesound(damagebuffer), coinsound(coinbuffer), healsound(healbuffer), gameoversound(gameoverbuffer) {
        fs::path path1 = asset_path("The-way-life-goes.ogg");
        require_loaded(music1.openFromFile(path1), path1);
        fs::path path2 = asset_path("music.ogg");
        require_loaded(music2.openFromFile(path2), path2);
        fs::path path3 = asset_path("yeat.ogg");
        require_loaded(music3.openFromFile(path3), path3);

        fs::path damagepath = asset_path("damagesound.wav");
        require_loaded(damagebuffer.loadFromFile(damagepath), damagepath);
        damagesound.setBuffer(damagebuffer);
        fs::path coinpath = asset_path("coinsound.wav");
        require_loaded(coinbuffer.loadFromFile(coinpath), coinpath);
        coinsound.setBuffer(coinbuffer);
        fs::path healpath = asset_path("healsound.wav");
        require_loaded(healbuffer.loadFromFile(healpath), healpath);
        healsound.setBuffer(healbuffer);
        fs::path gameoverpath = asset_path("gameoversound.wav");
        require_loaded(gameoverbuffer.loadFromFile(gameoverpath), gameoverpath);
        gameoversound.setBuffer(gameoverbuffer);

        damagesound.setVolume(5);
        coinsound.setVolume(10);
        healsound.setVolume(5);
        gameoversound.setVolume(5);
        music1.setVolume(10);
        music2.setVolume(10);
        music3.setVolume(10);
    }

    void play1() {
        music2.stop();
        music3.stop();  
        music1.play();
        current_track = 1;
    }

    void stop_music() {
        music1.stop();
        music2.stop();
        music3.stop();
    }

    void restart_music() {
        stop_music();
        play1();
    }

    void play2() {
        music1.stop();
        music3.stop();     
        music2.play();
        current_track = 2;
    }

    void play3() {
        music1.stop();
        music2.stop();
        music3.play();
        current_track = 3;
    }

    void switch_tracks() {
        if (current_track == 1) {
            play2();
        }
        else if (current_track == 2) {
            play3();
        }
        else {
            play1();
        }
    }
    
    void pause() {
        music1.pause();
        music2.pause();
        music3.pause();
        damagesound.pause();
        coinsound.pause();
        healsound.pause();
        gameoversound.pause();
    }

    void resume() {
        if (current_track == 1) music1.play();
        else if (current_track == 2) music2.play();
        else music3.play();
        if (damagesound.getStatus() == SoundSource::Status::Paused) damagesound.play();
        if (coinsound.getStatus() == SoundSource::Status::Paused) coinsound.play();
        if (healsound.getStatus() == SoundSource::Status::Paused) healsound.play();
    }

    void stop_effects() {
        damagesound.stop();
        coinsound.stop();
        healsound.stop();
        gameoversound.stop();
    }

    void playDamageSound(){
        damagesound.play();
    }

    void playCoinSound(){
        coinsound.play();
    }

    void playHealSound(){
        healsound.play();
    }

    void playGameoverSound(){
        gameoversound.play();
    }
};

class Menu {
private:
    Font font;
    Text *startText;
    Text *exitText;
    bool startSelected;
    RectangleShape background;
    Text controlsText;

public:
    Menu() : startSelected(true), controlsText(font,
        L"WASD — движение     N — сменить музыку     Esc / P — пауза", 32) {
        // Простой фон
        background.setSize(Vector2f(1920, 1080));
        background.setFillColor(Color(0, 0, 50, 200)); // Темно-синий полупрозрачный
        
        // Загрузка шрифта
        fs::path path = asset_path("PT_Sans-Regular.ttf");
        require_loaded(font.openFromFile(path), path);
        startText = new Text(font, "0", 100);
        exitText = new Text(font, "0", 100);
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
    }

    bool update() {
        // Просто ждем нажатия Enter
        if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
            return true; // Начать игру
        }
        return false; // Продолжить показывать меню
    }

    void draw(RenderWindow &window) {
        window.draw(background);
        window.draw(*startText);
        window.draw(*exitText);
        window.draw(controlsText);
    }
};

class Game {
    enum class State {
        Menu,
        Playing,
        Paused,
        GameOver
    };

private:
    RenderWindow window;
    Clock clock;
    float deltatime;

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

    void ProcessEvents(){
       while (const optional event = window.pollEvent()){
             if (event->is<Event::Closed>()){
                 window.close();
                 continue;
             }

             if (event->is<Event::FocusLost>()) {
                 focused = false;
                 if (state == State::Playing) pauseGame();
             } else if (event->is<Event::FocusGained>()) {
                 focused = true;
             }

             if (focused && window.isOpen()) {
                 if (const auto *keyPressed = event->getIf<Event::KeyPressed>()) {
                     handleKeyPressed(keyPressed->code);
                 }
             }
         }
    }

    void pauseGame() {
        state = State::Paused;
        sounds.pause();
    }

    void resumeGame() {
        clock.restart();
        sounds.resume();
        state = State::Playing;
    }

    void returnToMenu() {
        sounds.stop_music();
        sounds.stop_effects();
        state = State::Menu;
    }

    void handleKeyPressed(Keyboard::Key key) {
        if (state == State::Menu) {
            if (key == Keyboard::Key::Enter) {
                startGame();
            } else if (key == Keyboard::Key::Escape) {
                window.close();
            }
            return;
        }

        if (state == State::Playing) {
            if (key == Keyboard::Key::Escape || key == Keyboard::Key::P) {
                pauseGame();
            } else if (key == Keyboard::Key::N) {
                sounds.switch_tracks();
            }
            return;
        }

        if (state == State::Paused) {
            if (key == Keyboard::Key::Escape || key == Keyboard::Key::P || key == Keyboard::Key::Enter) {
                resumeGame();
            } else if (key == Keyboard::Key::M) {
                returnToMenu();
            } else if (key == Keyboard::Key::Q) {
                window.close();
            }
            return;
        }

        if (key == Keyboard::Key::R || key == Keyboard::Key::Enter) {
            startGame();
        } else if (key == Keyboard::Key::M) {
            returnToMenu();
        } else if (key == Keyboard::Key::Escape) {
            window.close();
        }
    }

    void update() {
        deltatime = clock.restart().asSeconds();
        player.move(deltatime);
        player.invincibility(deltatime);
        hud.update_background(deltatime);
        meteorit.move(deltatime);
        bonus.move(deltatime);
        coin.move(deltatime);
        checkCollisions();
    }

    void checkCollisions(){
        if (player.getGlobalBounds().findIntersection(meteorit.getGlobalBounds())) {
            if (player.damage()) {
                sounds.playDamageSound();
                hud.update_healthbar(player.get_health());
                player.respawn();
                if (player.get_health() == 0) {
                    state = State::GameOver;
                    sounds.stop_music();
                    sounds.playGameoverSound();
                    return;
                }
            }
        }

        if (player.getGlobalBounds().findIntersection(bonus.getGlobalBounds())) {
            int old_health = player.get_health();
            player.heal();
            hud.update_healthbar(player.get_health());
            if (player.get_health() > old_health) {
                sounds.playHealSound();
                bonus.respawn();
            }
        }

        if (player.getGlobalBounds().findIntersection(coin.getGlobalBounds())) {
            hud.update_scoredisplay();
            sounds.playCoinSound();
            coin.respawn();
        }        
    }

    void render() {
        window.clear(Color::Black);
        if (state == State::Menu) {
            menu.draw(window);
        } else if (state == State::Playing || state == State::Paused) {
            draw_game();
            if (state == State::Paused) pauseScreen.draw(window);
        } else {
            hud.draw(window);
            hud.draw_gameover(window, hud.get_score());
        }
        window.display();
    }

    void draw_game() {
        hud.draw(window);
        player.draw(window);
        meteorit.draw(window);
        bonus.draw(window);
        coin.draw(window);
    }

    void startGame() {
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
    Game(): window(VideoMode({1920, 1080}), L"Игра", sf::State::Fullscreen), deltatime(0), state(State::Menu) {
        window.setKeyRepeatEnabled(false);
        fs::path iconpath = asset_path("icon.png");
        Image icon;
        require_loaded(icon.loadFromFile(iconpath), iconpath);
        window.setIcon(icon);
    }

    void run(){
        while (window.isOpen()) {
            ProcessEvents();
            if (window.isOpen() && state == State::Playing) {
                update();
            }
            if (window.isOpen()) {
                render();
            }
        }
    }
};

int main(){
    try {
        g_asset_dir = executable_dir() / "assets";
        Game game;
        game.run();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
