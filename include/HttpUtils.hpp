#pragma once

#include <string>
#include <iostream>
#include <ctime>
#include <map>
#include <sstream>
#include <filesystem>  

// a helper to trim empty space
std::string trim_space(std::string str);

namespace fs = std::filesystem; // Alias for filesystem

// Function: map file extension to MIME type
std::string getMimeType(const std::string& path);

// Format time as GMT string for Last-Modified header
std::string formatTime(std::time_t t);