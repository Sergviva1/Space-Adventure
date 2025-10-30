#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
using namespace std;
using namespace sf;

class Player{
private:
    Texture texture;
    Sprite sprite;
    int current_frame = 0;
    float frametime = 0.f;
    float switch_time = 0.2f;
    int frame_width = 166;
    int frame_height = 130;
    float speed = 1000.f;

    void player_lock(){
        Vector2f posplayer = sprite.getPosition();
        if (posplayer.x > 1770) {
            sprite.setPosition({1770, posplayer.y});
        }
        if (posplayer.x < -20) {
            sprite.setPosition({-20, posplayer.y});
        }
        if (posplayer.y > 875) {
             sprite.setPosition({posplayer.x, 875});
        }
        if (posplayer.y < 197) {
            sprite.setPosition({posplayer.x, 197});
        }
    }

public:
    Player() : sprite(texture) {
        texture.loadFromFile("coding\\assets\\spritelist.png");
        sprite.setTexture(texture);
        sprite.setTextureRect(IntRect({0,0}, {166, 130}));
        sprite.setPosition({150.f, 50.f});
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
        window.draw(sprite);
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
        texturespace.loadFromFile("coding\\assets\\space.jpg");
        backgroundgame.setTexture(&texturespace);
        backgroundgame2.setTexture(&texturespace);
        backgroundgame.setSize(Vector2f{1920,1080});
        backgroundgame2.setSize(Vector2f{1920,1080});
        backgroundgame2.setPosition(Vector2f {1920,0});
    }

    void background_move(float deltatime){
        backgroundgame.move({-backgroundspeed * deltatime,0});
        pos = backgroundgame.getPosition();
        if (pos.x<-1920) backgroundgame.setPosition({1920,pos.y});
        backgroundgame2.move({-backgroundspeed * deltatime,0});
        pos = backgroundgame2.getPosition();
        if (pos.x<-1920) backgroundgame2.setPosition({1920,pos.y});
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
        gamepaneltexture.loadFromFile("coding\\assets\\gamepanel.png");
        GamePanelshape.setTexture(&gamepaneltexture);
        GamePanelshape.setSize(Vector2f{1920,720});
        GamePanelshape.setPosition(Vector2f{0,0});
    }

    void draw(RenderWindow &window){
        window.draw(GamePanelshape);
    }
};

class Tracks {
private:
    Music music1;
    Music music2;
    Music music3;
    int current_track;
public:
    Tracks() : current_track(1) {
        music1.openFromFile("coding\\assets\\The-way-life-goes.ogg");
        music2.openFromFile("coding\\assets\\music.ogg");
        music3.openFromFile("coding\\assets\\yeat.ogg");
    }

    void play1() {
        music2.stop();
        music3.stop();
        music1.setVolume(10);
        music1.play();
        current_track = 1;
    }

    void play2() {
        music1.stop();
        music3.stop();
        music2.setVolume(10);
        music2.play();
        current_track = 2;
    }

    void play3() {
        music1.stop();
        music2.stop();
        music3.setVolume(10);
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
    
    void check_press(){
        if (Keyboard::isKeyPressed(Keyboard::Key::N)) {
        switch_tracks();
        }
    }
};

class Enemies {
private:
    Texture enemy_texture;
    Sprite *enemy_sprite;
    float speed = 300;
public:
    Enemies () {
        enemy_texture.loadFromFile("coding\\assets\\meteor.png");
        enemy_sprite = new Sprite(enemy_texture);
        enemy_sprite->setPosition({1980,197 + rand() % 875});
        enemy_sprite->setScale({0.15f,0.15f});
    }

    Vector2f get_position() {
        return enemy_sprite->getPosition();
    }

    void move(float deltatime) {
        enemy_sprite->move({-speed * deltatime, 0});
        if (get_position().x < -200 ) {
            enemy_sprite->setPosition({1980,197 + rand() % 875});
        }
    }

    void draw(RenderWindow &window) {
        window.draw(*enemy_sprite);
    }
};

int main(){

    // Отрисовка окна и иконки
    RenderWindow window(VideoMode::getDesktopMode (), L"Игра");
    Image icon ("coding\\assets\\icon.png");
    window.setIcon(icon);

    Player player;
    Background background;
    GamePanel gamepanel;
    Tracks myzika;
    Enemies meteorit;
    
    myzika.play1();

    Clock clock;
    float deltatime;
    srand(time(NULL));

    // Основной цикл
    while (window.isOpen()){
        deltatime = clock.restart().asSeconds();
        while (const optional event = window.pollEvent()){
            if (event->is<Event::Closed>()){
                window.close();
            }  
        }
        
        myzika.check_press();
        player.move(deltatime);
        background.background_move(deltatime);
        meteorit.move(deltatime);
       
        window.clear(Color::Black);
        background.draw(window);
        gamepanel.draw(window);
        player.draw(window);
        meteorit.draw(window);
        window.display();
    }
}