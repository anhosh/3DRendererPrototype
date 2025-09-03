#pragma once

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

inline std::optional<fs::path> locateDirectory(const fs::path& relativePath) {
  fs::path dir = fs::current_path();
  while (!fs::is_directory(dir / relativePath)) {
    if (dir == "/") {
      return std::nullopt;
    }
    dir = dir.parent_path();
  }
  return dir / relativePath;
}
