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
