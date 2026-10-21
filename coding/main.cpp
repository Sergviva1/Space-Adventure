#include "Game.hpp"

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
