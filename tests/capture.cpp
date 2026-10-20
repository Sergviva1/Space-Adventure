// Сохранены реальные экраны рендера; позиции объектов заданы для наглядности.
#define main game_entry_point
#include "../coding/main.cpp"
#undef main

int main() {
    try {
        g_asset_dir = executable_dir() / "assets";
        const fs::path output = executable_dir() / "docs" / "screenshots";
        fs::create_directories(output);
        Game game(fs::path{});
        game.record.best = 12;
        game.window.setVisible(false);
        auto capture = [&](const char* filename) {
            // Заполнены оба буфера: драйвер может читать предыдущий после display().
            game.render();
            game.render();
            Texture frame;
            if (!frame.resize(game.window.getSize())) throw runtime_error("Cannot allocate screenshot texture");
            frame.update(game.window);
            if (!frame.copyToImage().saveToFile(output / filename)) {
                throw runtime_error(string("Cannot save screenshot: ") + filename);
            }
        };
        capture("menu.png");
        game.startGame();
        game.sounds.stop_music();
        game.player.sprite.setPosition({450, 550});
        game.meteorit.spriteobj->setPosition({1200, 450});
        game.coin.spriteobj->setPosition({900, 650});
        game.bonus.spriteobj->setPosition({1500, 800});
        for (int i = 0; i < 7; ++i) game.hud.update_scoredisplay();
        capture("gameplay.png");
        game.pauseGame();
        capture("pause.png");
        game.state = Game::State::GameOver;
        game.hud.update_healthbar(0);
        capture("gameover.png");
        cout << "Screenshots saved to " << output << endl;
        return 0;
    } catch (const exception& error) {
        cerr << error.what() << endl;
        return 1;
    }
}
