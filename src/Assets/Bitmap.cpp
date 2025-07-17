#include <Assets/Bitmap.hpp>

#include <stb_image.h>

#include <filesystem>

namespace fs = std::filesystem;

Expected<Bitmap> Bitmap::fromFile(const fs::path& fileName) {
  stbi_set_flip_vertically_on_load(true);
  int32_t width, height, channels;
  uint8_t* loadedData = stbi_load(fileName.c_str(), &width, &height, &channels, 0);
  if (!loadedData) {
    stbi_image_free(loadedData);
    return std::unexpected(std::format("Bitmap file was not found: {}", fileName.c_str()));
  }

  Expected ret = Bitmap::fromMemory(std::span(loadedData, static_cast<uint32_t>(width * height * channels)),
                                    glm::uvec2(width, height), static_cast<uint32_t>(channels));
  stbi_image_free(loadedData);
  ret.value().mFilePath = fileName;
  return ret;
}

Expected<Bitmap> Bitmap::fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size, uint32_t channels) {
  if (bytes.size() != size.x * size.y * channels) {
    return std::unexpected(std::format("Bitmap size does not match data length: {} != {} [width({}) * height({}) * channels({})]",
                                            bytes.size(), size.x * size.y * channels, size.x, size.y, channels));
  }

  Bitmap ret;
  ret.mSize = size;
  ret.mChannels = channels;
  ret.mData.assign(bytes.begin(), bytes.end());
  return ret;
}

