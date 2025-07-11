#include <Graphics/Bitmap.hpp>

#include <stb_image.h>

#include <filesystem>

namespace fs = std::filesystem;

Expected<Bitmap> Bitmap::fromFile(const fs::path& path) {
  stbi_set_flip_vertically_on_load(true);
  int32_t width, height, channels;
  uint8_t* loadedData = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (!loadedData) {
    stbi_image_free(loadedData);
    return std::unexpected(std::format("Bitmap file was not found: {}", path.c_str()));
  }

  auto ret = Bitmap::fromMemory(std::span(loadedData, static_cast<uint32_t>(width * height * channels)),
                                   glm::uvec2(width, height), static_cast<uint32_t>(channels));
  stbi_image_free(loadedData);
  ret.value().filePath = path;
  return ret;
}

Expected<Bitmap> Bitmap::fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size, uint32_t channels) {
  if (bytes.size() != size.x * size.y * channels) {
    return std::unexpected(std::format("Bitmap size does not match data length: {} != {} * {} * {}",
                                            bytes.size(), size.x, size.y, channels));
  }

  Bitmap ret;
  ret.mSize = size;
  ret.mChannels = channels;
  ret.data.assign(bytes.begin(), bytes.end());
  return ret;
}

