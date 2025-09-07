#include <GUI.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void initialiseImGui(NotNull<GLFWwindow> window) {
  ZoneScoped;

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

void shutdownImGui() {
  ZoneScoped;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void runImGui(AppState& state) {
  ZoneScoped;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  state.currentDemo->gui(state);
  ImGui::Render();
}

void renderImGui() {
  ZoneScoped;

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
