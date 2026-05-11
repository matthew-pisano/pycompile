//
// Created by matthew on 3/1/26.
//

#include "utils.h"

#include <fstream>
#include <glob.h>
#include <mlir/IR/Location.h>
#include <sstream>
#include <stdexcept>


std::vector<std::string> resolveWildcards(const std::vector<std::string>& rawPaths) {
    std::vector<std::string> resolvedPaths;
    for (const auto& path : rawPaths) {
        // Use glob to resolve wildcards
        glob_t globResult{};
        if (glob(path.c_str(), GLOB_TILDE, nullptr, &globResult) == 0) {
            for (size_t i = 0; i < globResult.gl_pathc; ++i)
                resolvedPaths.emplace_back(globResult.gl_pathv[i]);
            globfree(&globResult);
        } else
            // If glob fails, add the original path
            resolvedPaths.push_back(path);
    }
    return resolvedPaths;
}


std::string readFileString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file '" + filename + "'");

    std::ostringstream oss;
    oss << file.rdbuf();
    file.close();

    return oss.str();
}


void writeFileString(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file '" + filename + "'");

    file << content;
    file.close();
}


std::string getMLIRModuleName(const mlir::OwningOpRef<mlir::ModuleOp>& mlirModule) {
    mlir::Location loc = mlirModule.get().getLoc();
    if (const mlir::FileLineColLoc fileLoc = mlir::dyn_cast<mlir::FileLineColLoc>(loc))
        return fileLoc.getFilename().str();
    throw std::runtime_error("Unable to parse PyIR module name");
}
