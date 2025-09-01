#pragma once

#include <expected>
#include <string>

template <typename T>
using Expected = std::expected<T, std::string>;
