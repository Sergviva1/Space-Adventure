#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <random>
#include <string>
using namespace std;
using namespace sf;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(197,955);

class GameObjects{
protected:
    Texture textureobj;
    Sprite *spriteobj;
    float speed;
public:
    GameObjects(string img, float spd) : speed(spd){
        textureobj.loadFromFile(img);
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
        texture.loadFromFile("coding\\assets\\spritelist.png");
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

    bool get_can_be_hit() {
        return can_be_hit;
    }

    void damage() {
        if (can_be_hit == true) {
            health = health - 1;
            can_be_hit = false;
        }
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
    
    void reset_can_be_hit() {
        can_be_hit = true;
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

class Enemies : public GameObjects {
public:
    Enemies() : GameObjects("coding/assets/meteor.png", 300) {
        spriteobj->setScale({0.15f, 0.15f});
    }
};

class GameOver {
private:
    Texture GameOver_Texture;
    Sprite *GameOver_Sprite;
public:
    GameOver() {
        GameOver_Texture.loadFromFile("coding\\assets\\gameover.png");
        GameOver_Sprite = new Sprite (GameOver_Texture);
        GameOver_Sprite->setScale({0.4f,0.4f});
        GameOver_Sprite->setPosition({650,339});
    }

    void draw(RenderWindow &window){
        window.draw(*GameOver_Sprite);
    }

    ~GameOver() {
        delete GameOver_Sprite;
    }
};

class HealthBar {
private:
    Texture Texture_HealthBar;
    Sprite *Sprite_HealthBar;
public:
    HealthBar() {
        Texture_HealthBar.loadFromFile("coding\\assets\\healthbar.png");
        Sprite_HealthBar = new Sprite(Texture_HealthBar);
        Sprite_HealthBar->setPosition({0,0});
        Sprite_HealthBar->setScale({0.5f,0.5f});
        update(3);

    }

    void update(int health) {
        if (health == 3) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 0}, {312, 104}));
        } else if (health == 2) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 107}, {312, 104}));
        } else if (health == 1) {
            Sprite_HealthBar->setTextureRect(IntRect({0, 207}, {312, 104}));
        }
    }

    void draw (RenderWindow &window) {
        window.draw(*Sprite_HealthBar);
    }

    ~HealthBar() {
        delete Sprite_HealthBar;
    }
};

class Bonus : public GameObjects{
public:
    Bonus() : GameObjects("coding/assets/bonus.png",300){
    }
};

class ScoreDisplay {
private:
    Font font;
    Text *score;
    float score_value = 0.0f;
public:
    ScoreDisplay() {
        font.openFromFile("C:/Windows/Fonts/Arial.ttf");
        score = new Text(font, "0", 100);
        score->setPosition({3,50});
        score->setScale({0.7f,0.7f});
        score->setFillColor(Color::Black);
    }

    void score_plus(float deltatime){
        score_value += deltatime * 30;
        score->setString(to_string((int)score_value));
    }

    float get_score_value() {
        return score_value;
    }

    void draw(RenderWindow &window) {
        window.draw(*score);
    }

    ~ScoreDisplay(){
        delete score;
    }
};

int main(){

    // Отрисовка окна и иконки
    RenderWindow window(VideoMode({1920,1080}), L"Игра", State::Fullscreen);
    Image icon ("coding\\assets\\icon.png");
    window.setIcon(icon);

    Player player;
    Background background;
    GamePanel gamepanel;
    Tracks myzika;
    Enemies meteorit;
    GameOver gameover;
    HealthBar healthbar;
    Bonus bonus;
    ScoreDisplay score;
    
    myzika.play1();

    Clock clock;
    float deltatime;

    // Основной цикл
    while (window.isOpen()){
        deltatime = clock.restart().asSeconds();
        while (const optional event = window.pollEvent()){
            if (event->is<Event::Closed>()){
                window.close();
            }  
        }
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
            window.close();
        }

        player.move(deltatime);
        player.invincibility(deltatime);
        background.background_move(deltatime);
        meteorit.move(deltatime);
        bonus.move(deltatime);
        myzika.check_press();
        score.score_plus(deltatime);

        if (player.getGlobalBounds().findIntersection(meteorit.getGlobalBounds())) {
            player.damage();
            healthbar.update(player.get_health());
            player.respawn();
            if (player.get_health() == 0) {
                gameover.draw(window);
                window.display();   
                sleep(seconds(2));
                window.close();
            }
        }

        if (player.getGlobalBounds().findIntersection(bonus.getGlobalBounds())) {
            int old_health = player.get_health();
            player.heal();
            healthbar.update(player.get_health());
            if (player.get_health() > old_health) {
                bonus.respawn();
            };
        }
        
        window.clear(Color::Black);
        background.draw(window);
        gamepanel.draw(window);
        player.draw(window);
        meteorit.draw(window);
        healthbar.draw(window);
        bonus.draw(window);
        score.draw(window);
        window.display();
    }
}