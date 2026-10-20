// Проверены внутренние инварианты без изменения публичного API игры.
#define main game_entry_point
#include "../coding/main.cpp"
#undef main
#include "temp-directory.hpp"

void check(bool condition, const char* message) {
    if (!condition) throw runtime_error(message);
}

int main() {
    try {
        TemporaryDirectory temporary;
        const fs::path recordPath = temporary.path / L"Рекорд с пробелами" / "highscore.txt";
        HighScore record(recordPath);
        check(record.get() == 0, "missing high score");
        record.update(7);
        check(!record.save_failed() && HighScore(recordPath).get() == 7, "high score persisted");
        record.update(3);
        check(HighScore(recordPath).get() == 7, "lower score must not overwrite best");

        // Блокировка назначения не повредила прежний файл при ошибке замены.
        const HANDLE locked = CreateFileW(recordPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                          nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        check(locked != INVALID_HANDLE_VALUE, "lock score file");
        record.update(9);
        const bool failedSafely = record.save_failed() && record.get() == 9 && HighScore(recordPath).get() == 7;
        CloseHandle(locked);
        check(failedSafely, "failed save preserves previous score");
        record.update(8);
        check(!record.save_failed() && HighScore(recordPath).get() == 9, "retry preserves in-memory best");
        record.update(10);
        check(HighScore(recordPath).get() == 10, "higher score replaces saved record");
        for (const string invalid : {"", "-1", "123oops", "1 2", "99999999999999999999999999999999999999"}) {
            ofstream(recordPath, ios::trunc) << invalid;
            check(HighScore(recordPath).get() == 0, "corrupt score ignored");
        }
        ofstream(recordPath, ios::trunc) << numeric_limits<int>::max();
        check(HighScore(recordPath).get() == numeric_limits<int>::max(), "maximum score loads");
        ofstream(recordPath, ios::trunc) << "broken";
        HighScore reset(recordPath);
        reset.update(4);
        check(HighScore(recordPath).get() == 4, "corrupt score replaced by new best");
        const fs::path blocker = temporary.path / "not-a-directory";
        ofstream(blocker) << "blocker";
        HighScore unavailable(blocker / "highscore.txt");
        unavailable.update(5);
        check(unavailable.save_failed() && unavailable.get() == 5, "unwritable storage is nonfatal");

        g_asset_dir = executable_dir() / "assets";
        const fs::path gameRecordPath = temporary.path / "game-highscore.txt";
        Game game(gameRecordPath);
        game.window.setVisible(false);
        game.startGame();
        check(game.state == Game::State::Playing, "start state");
        check(game.player.get_health() == 3 && game.hud.get_score() == 0, "initial values");
        check(game.player.texture.getSize() == Vector2u(166, 260), "player atlas: two vertical frames");
        check(game.hud.healthbar.Texture_HealthBar.getSize().x >= 312 &&
              game.hud.healthbar.Texture_HealthBar.getSize().y >= 311, "health bar atlas bounds");
        check(game.coin.getGlobalBounds().size.x <= 96.01f && game.coin.getGlobalBounds().size.y <= 96.01f,
              "coin display size independent of source resolution");
        check(game.bonus.getGlobalBounds().size.x <= 92.01f && game.bonus.getGlobalBounds().size.y <= 92.01f,
              "bonus display size independent of source resolution");
        check(game.meteorit.getGlobalBounds().size.x <= 125.01f && game.meteorit.getGlobalBounds().size.y <= 125.01f,
              "meteor display size independent of source resolution");
        check(game.hud.gamepanel.GamePanelshape.getSize() == Vector2f(1920, 1080), "full-screen panel");
        auto& bar = game.hud.healthbar;
        bar.update(1);
        const int oneLifeWidth = bar.Sprite_HealthBar->getTextureRect().size.x;
        check(bar.Sprite_HealthBar->getColor().a > 0, "one life must not appear empty");
        bar.update(2);
        check(bar.Sprite_HealthBar->getTextureRect().size.x > oneLifeWidth, "two lives fill more than one");
        bar.update(0);
        check(bar.Sprite_HealthBar->getColor().a == 0, "zero lives empty");
        bar.update(3);

        game.player.sprite.setPosition({3000, 3000});
        game.player.player_lock();
        check(game.player.sprite.getPosition() == Vector2f(1780, 955), "bottom right clamp");
        game.player.sprite.setPosition({-3000, -3000});
        game.player.player_lock();
        check(game.player.sprite.getPosition() == Vector2f(-25, 197), "top left clamp");
        game.player.respawn();

        auto overlap = [&](GameObjects& object) {
            object.spriteobj->setPosition(game.player.sprite.getPosition());
        };
        overlap(game.bonus);
        const auto bonusPosition = game.bonus.get_position();
        game.checkCollisions();
        check(game.bonus.get_position() == bonusPosition, "full health must preserve bonus");
        overlap(game.coin);
        game.checkCollisions();
        game.checkCollisions();
        check(game.hud.get_score() == 1, "coin collected once");
        check(game.record.get() == 1 && HighScore(gameRecordPath).get() == 1, "coin updates persisted record");

        overlap(game.meteorit);
        game.checkCollisions();
        check(game.player.get_health() == 2, "damage");
        game.player.sprite.setPosition({400, 400});
        overlap(game.meteorit);
        game.checkCollisions();
        check(game.player.get_health() == 2, "invincibility health");
        check(game.player.sprite.getPosition() == Vector2f(400, 400), "invincibility position");
        game.meteorit.respawn();
        overlap(game.bonus);
        game.checkCollisions();
        check(game.player.get_health() == 2, "healing blocked during invincibility");
        game.player.invincibility(2.1f);
        game.checkCollisions();
        check(game.player.get_health() == 3, "healing after invincibility");

        game.handleKeyPressed(Keyboard::Key::N);
        check(game.sounds.current_track == 2, "track switch");
        game.handleKeyPressed(Keyboard::Key::P);
        check(game.state == Game::State::Paused, "pause state");
        check(game.sounds.music2.getStatus() == SoundSource::Status::Paused, "music paused");
        game.handleKeyPressed(Keyboard::Key::Enter);
        check(game.state == Game::State::Playing, "resume state");
        check(game.sounds.current_track == 2, "resume track");
        check(game.sounds.music2.getStatus() == SoundSource::Status::Playing, "music resumed");

        sf::sleep(sf::milliseconds(100));
        game.update();
        check(game.deltatime <= 0.05f, "frame time capped");
        for (int i = 0; i < 3; ++i) {
            game.player.invincibility(2.1f);
            overlap(game.meteorit);
            game.checkCollisions();
        }
        check(game.state == Game::State::GameOver, "game over state");
        check(game.player.get_health() == 0, "zero health");
        check(game.sounds.music2.getStatus() == SoundSource::Status::Stopped, "game over stops music");
        game.handleKeyPressed(Keyboard::Key::R);
        check(game.state == Game::State::Playing, "restart state");
        check(game.player.get_health() == 3 && game.player.can_be_hit, "restart health");
        check(game.hud.get_score() == 0 && game.sounds.current_track == 1, "restart score and music");
        check(game.player.sprite.getPosition() == Vector2f(150, 550), "restart position");
        check(game.record.get() == 1, "restart preserves record");
        game.handleKeyPressed(Keyboard::Key::P);
        game.handleKeyPressed(Keyboard::Key::M);
        check(game.state == Game::State::Menu, "return to menu");
        game.handleKeyPressed(Keyboard::Key::Enter);
        game.handleKeyPressed(Keyboard::Key::Escape);
        game.handleKeyPressed(Keyboard::Key::Q);
        check(!game.window.isOpen(), "quit from pause");

        ScoreDisplay scoreLimit;
        scoreLimit.score_value = numeric_limits<int>::max();
        scoreLimit.score_plus();
        check(scoreLimit.get_score() == numeric_limits<int>::max(), "score does not overflow");

        Background background;
        for (int i = 0; i < 10000; ++i) {
            background.move(0.05f);
            const float distance = abs(background.backgroundgame.getPosition().x -
                                       background.backgroundgame2.getPosition().x);
            check(abs(distance - 1920.f) < 0.01f, "background seamless wrap");
        }
        cout << "Smoke checks passed" << endl;
        return 0;
    } catch (const exception& error) {
        cerr << error.what() << endl;
        return 1;
    }
}
