#pragma once

#include <Util/Registry.hpp>

template <typename Asset>
using AssetHandle = Registry<Asset>::Handle;

