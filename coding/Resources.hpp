#pragma once

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

extern mt19937 gen;
extern uniform_int_distribution<> distrib;
extern fs::path g_asset_dir;

fs::path asset_path(const fs::path &filename);
void require_loaded(bool loaded, const fs::path &path);
fs::path executable_dir();
fs::path high_score_path();
void fit_sprite(Sprite& sprite, Vector2f maximum);
