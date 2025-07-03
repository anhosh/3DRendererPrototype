#include <Bitmap.hpp>

#include <Paths.hpp>

#include <stb_image.h>

#include <filesystem>

namespace fs = std::filesystem;

static fs::path sTexturesDir = "textures";

bool locateTextures() {
  if (std::optional<fs::path> texturesDir = locateDirectory("textures")) {
    sTexturesDir = texturesDir.value();
    return true;
  }
  return false;
}

std::expected<Bitmap, std::string> Bitmap::fromFile(const fs::path& fileName) {
  int32_t width, height, channels;
  uint8_t* loadedData = stbi_load((sTexturesDir / fileName).c_str(), &width, &height, &channels, 0);
  if (!loadedData) {
    stbi_image_free(loadedData);
    return std::unexpected(std::format("Bitmap file was not found: {}", (sTexturesDir / fileName).c_str()));
  }

  auto ret = Bitmap::fromMemory(std::span(loadedData, static_cast<uint32_t>(width * height * channels)),
                                   glm::uvec2(width, height), static_cast<uint32_t>(channels));
  stbi_image_free(loadedData);
  return ret;
}

std::expected<Bitmap, std::string> Bitmap::fromMemory(std::span<const uint8_t> bytes, glm::uvec2 size, uint32_t channels) {
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

