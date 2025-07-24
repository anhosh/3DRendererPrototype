#pragma once
#include <GLFW/glfw3.h>
#include <Util/Log.hpp>

class ScopedTimer {
public:
  explicit ScopedTimer(std::string&& name)
    : name(std::move(name))
    , timeStart(glfwGetTime())
  {}

  ~ScopedTimer() {
    double timeEnd = glfwGetTime();
    LOG_INFO("{} - duration: {}s", name, timeEnd - timeStart);
  }

private:
  std::string name;
  double timeStart;
};
