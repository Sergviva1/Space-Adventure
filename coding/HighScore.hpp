#pragma once

#include "Resources.hpp"

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
