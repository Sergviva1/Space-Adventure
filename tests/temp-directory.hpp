// Временные данные тестов не затронули профиль игрока.
#pragma once

class TemporaryDirectory {
public:
    fs::path path;

    TemporaryDirectory() : path(fs::temp_directory_path() /
        ("SpaceAdventure-tests-" + to_string(GetCurrentProcessId()) + "-" + to_string(GetTickCount64()))) {
        if (!fs::create_directory(path)) throw runtime_error("Cannot create isolated test directory");
    }

    ~TemporaryDirectory() {
        error_code error;
        fs::remove_all(path, error);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
};
