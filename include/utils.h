//
// Created by matthew on 3/1/26.
//

#ifndef PYCOMPILE_UTILS_H
#define PYCOMPILE_UTILS_H
#include <string>
#include <mlir/IR/BuiltinOps.h>


/**
 * Error with included location information.
 */
class PyCompileError : public std::exception {
public:
    PyCompileError(const std::string& msg, const std::string& moduleName, const size_t lineno = 0,
                   const size_t offset = 0) {
        this->msg = std::format("{}:{}:{}: error: {}", moduleName, lineno, offset, msg);
    }

    PyCompileError(const std::string& msg, const std::string& moduleName, const std::optional<size_t> lineno,
                   const size_t offset = 0) {
        size_t resolvedLineno = lineno.has_value() ? *lineno : 0;
        this->msg = std::format("{}:{}:{}: error: {}", moduleName, resolvedLineno, offset, msg);
    }

    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }

private:
    std::string msg;
};


/**
 * Helper function to read the contents of a file into a string.
 * @param filename The path to the file to read.
 * @return A string containing the contents of the file.
 * @throws std::runtime_error if the file cannot be opened or read.
 */
std::string readFileString(const std::string& filename);


/**
 * Helper function to write contents to a file
 * @param filename The file to write to.
 * @param content The content to write to the file.
 */
void writeFileString(const std::string& filename, const std::string& content);


/**
 * Extracts the name from an MLIR module.
 * @param mlirModule The module to name.
 * @return The module name.
 * @throw runtime_error if no name could be determined.
 */
std::string getMLIRModuleName(const mlir::OwningOpRef<mlir::ModuleOp>& mlirModule);

#endif //PYCOMPILE_UTILS_H
