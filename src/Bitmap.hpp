#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool locateTextures();

class Bitmap {
public:
  static std::expected<Bitmap, std::string> fromFile(const fs::path& fileName);
  static std::expected<Bitmap, std::string> fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size, uint32_t channels);

  [[nodiscard]] glm::uvec2 size() const { return mSize; }
  [[nodiscard]] uint32_t channels() const { return mChannels; }

public:
  std::vector<uint8_t> data;

private:
  glm::uvec2 mSize = glm::uvec2(0);
  uint32_t mChannels = 0;
};
