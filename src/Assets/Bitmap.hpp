#pragma once

#include <Util/Expected.hpp>

#include <filesystem>
#include <span>
#include <vector>

class Bitmap {
public:
  [[nodiscard]] static Expected<Bitmap> fromFile(const std::filesystem::path& fileName);
  [[nodiscard]] static Expected<Bitmap> fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size,
                                                                     uint32_t channels);

  [[nodiscard]] glm::uvec2 size() const { return mSize; }
  [[nodiscard]] uint32_t channels() const { return mChannels; }

public:
  std::filesystem::path filePath;
  std::vector<uint8_t> data;

private:
  glm::uvec2 mSize = glm::uvec2(0);
  uint32_t mChannels = 0;
};
