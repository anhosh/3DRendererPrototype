#pragma once

#include <GLFW/glfw3.h>

#include <concepts>
#include <functional>

template <std::invocable<> Fn>
double timedBlock(Fn&& block) {
  const double timeBefore = glfwGetTime();
  std::invoke(std::forward<Fn>(block));
  const double timeAfter = glfwGetTime();
  return timeAfter - timeBefore;
}
