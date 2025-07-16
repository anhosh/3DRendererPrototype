#pragma once
#include <Application.hpp>

void initialiseImGui(GLFWwindow* window);
void shutdownImGui();
void runImGui(AppState& state);
void renderImGui();
