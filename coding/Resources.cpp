#include "Resources.hpp"

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

// Размер спрайта ограничен игровой областью, а не разрешением исходной картинки.
void fit_sprite(Sprite& sprite, Vector2f maximum)
{
    const Vector2f size = sprite.getLocalBounds().size;
    const float scale = min(maximum.x / size.x, maximum.y / size.y);
    sprite.setScale({scale, scale});
}
