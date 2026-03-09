//
// Created by matthew on 3/1/26.
//

#ifndef PYCOMPILE_UTILS_H
#define PYCOMPILE_UTILS_H
#include <string>

/**
 * Helper function to read the contents of a file into a string.
 * @param filename The path to the file to read.
 * @return A string containing the contents of the file.
 * @throws std::runtime_error if the file cannot be opened or read.
 */
std::string readFileString(const std::string& filename);

#endif //PYCOMPILE_UTILS_H
