#pragma once

struct AppState;

void initialiseImGui(GLFWwindow* window);
void shutdownImGui();
void runImGui(AppState& state);
void renderImGui();
