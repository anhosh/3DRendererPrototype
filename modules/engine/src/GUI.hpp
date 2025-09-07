#pragma once

#include <AppState.hpp>
#include <Util/NotNull.hpp>

struct AppState;

void initialiseImGui(NotNull<GLFWwindow> window);
void shutdownImGui();
void runImGui(AppState& state);
void renderImGui();
