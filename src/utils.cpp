//
// Created by matthew on 3/1/26.
//

#include "utils.h"

#include <stdexcept>
#include <fstream>
#include <sstream>

std::string readFileString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename);

    std::ostringstream oss;
    oss << file.rdbuf();
    file.close();

    return oss.str();
}


void writeFileString(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename);

    file << content;
    file.close();
}
