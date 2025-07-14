#pragma once

#include <Util/Expected.hpp>

#include <filesystem>
#include <span>
#include <vector>

class Bitmap {
public:
  [[nodiscard]] static Expected<Bitmap> fromFile(const std::filesystem::path& fileName);
  [[nodiscard]] static Expected<Bitmap> fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size, uint32_t channels);

  [[nodiscard]] const std::vector<uint8_t>& data() const { return mData; }
  [[nodiscard]] const uint8_t* bytes() const { return mData.data(); }
  [[nodiscard]] size_t size_bytes() const { return mData.size(); }
  [[nodiscard]] glm::uvec2 size() const { return mSize; }
  [[nodiscard]] uint32_t channels() const { return mChannels; }
  [[nodiscard]] const std::filesystem::path& filePath() const { return mFilePath; }

private:
  std::vector<uint8_t> mData;
  glm::uvec2 mSize = glm::uvec2(0);
  uint32_t mChannels = 0;
  std::filesystem::path mFilePath;
};
