#pragma once

#include <Util/Registry.hpp>

template <typename Asset>
using AssetHandle = typename Registry<Asset>::Handle;

