#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <iostream>

static std::string load_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to load file: " << path << "\n";
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
