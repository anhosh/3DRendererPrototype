#pragma once

template <typename... OverloadTypes>
struct Visitor : OverloadTypes... {
  using OverloadTypes::operator()...;
};
