#include <GUI.hpp>

#include <AppState.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ranges>

constexpr ImGuiColorEditFlags lightColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

void guiDebug(const AppState& state) {
  ImGui::Text("FPS: %.03f", 1.0 / (state.currentFrameTime - state.lastFrameTime));
  ImGui::Text("Frame duration: %.03f ms", (state.currentFrameTime - state.lastFrameTime) * 1000.0);
  ImGui::Text("Scene draw: %.03f ms", state.lastSceneRenderDuration * 1000.0);
  ImGui::Text("GUI draw: %.03f ms", state.lastGuiRenderDuration * 1000.0);
  ImGui::Text("Window size: %ux%u", state.windowSize.x, state.windowSize.y);

  if (ImGui::CollapsingHeader("Debug")) {
    ImGui::Indent();

    static constexpr auto sceneRenderModeNames = std::array {
      "Normal",
      "Wireframe",
      "Surface normal",
      "Surface depth",
    };
    ImGui::Combo("Render mode", reinterpret_cast<int32_t*>(&state.renderingEngine->sceneRenderMode),
                 sceneRenderModeNames.data(), sceneRenderModeNames.size());

    ImGui::Checkbox("Draw vertex normals", &state.renderingEngine->bVisualiseVertexNormals);


    ImGui::Unindent();
  }
}

void guiCamera(AppState& state) {
  if (ImGui::CollapsingHeader("Camera")) {
    ImGui::Indent();
    Camera& camera = state.mainCamera;

    ImGui::DragFloat("Movement speed", &camera.speed, 0.001f, 0.0f, 5.0f);
    ImGui::DragFloat("FOV", &camera.fov, 0.1f, 10.0f, 120.0f);
    ImGui::DragFloat("Near", &camera.near, 0.01f, 0.01f, 10.0f);
    ImGui::DragFloat("Far", &camera.far, 0.01f, 10.0f, 1000.0f);

    ImGui::Spacing();

    ImGui::Checkbox("Flashlight following camera", &state.bFlashlightFollowCamera);
    ImGui::Checkbox("Back mirror", &state.bBackMirror);

    ImGui::Unindent();
  }
}

void guiLight(AppState& state) {
  if (ImGui::CollapsingHeader("Light")) {
    ImGui::Indent();

    if (ImGui::CollapsingHeader("Directional lights")) {
      ImGui::Indent();

      size_t index = 0;
      for (DirectionalLight& directionalLight : std::ranges::views::values(state.scene->directionalLights)) {
        if (ImGui::CollapsingHeader(std::format("{}##sl{}", directionalLight.name, index).c_str())) {
          ImGui::Indent();

          ImGui::DragFloat3(("Direction##dl" + std::to_string(index)).c_str(), glm::value_ptr(directionalLight.direction), 0.001f, -1.0f, 1.0f);
          ImGui::Spacing();

          ImGui::ColorPicker3(("Ambient##dl" + std::to_string(index)).c_str(), glm::value_ptr(directionalLight.colors.ambient), lightColorEditFlags);
          ImGui::ColorPicker3(("Diffuse##dl" + std::to_string(index)).c_str(), glm::value_ptr(directionalLight.colors.diffuse), lightColorEditFlags);
          ImGui::ColorPicker3(("Specular##dl" + std::to_string(index)).c_str(), glm::value_ptr(directionalLight.colors.specular), lightColorEditFlags);

          ImGui::Unindent();
        }
        ++index;
      }

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Point lights")) {
      ImGui::Indent();

      size_t index = 0;
      for (PointLight& pointLight : std::ranges::views::values(state.scene->pointLights)) {
        if (ImGui::CollapsingHeader(std::format("{}##sl{}", pointLight.name, index).c_str())) {
          ImGui::Indent();

          ImGui::DragFloat3(("Position##pl" + std::to_string(index)).c_str(), glm::value_ptr(pointLight.position), 0.001f, -1.0f, 1.0f);
          ImGui::Spacing();

          ImGui::DragFloat("Constant", &pointLight.constant, 0.1f, 1.0f, 100.0f);
          ImGui::DragFloat("Linear", &pointLight.linear, 0.01f, 0.01f, 10.0f);
          ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.001f, 0.01f, 1.0f);
          ImGui::Spacing();

          ImGui::ColorPicker3(("Ambient##pl" + std::to_string(index)).c_str(), glm::value_ptr(pointLight.colors.ambient), lightColorEditFlags);
          ImGui::ColorPicker3(("Diffuse##pl" + std::to_string(index)).c_str(), glm::value_ptr(pointLight.colors.diffuse), lightColorEditFlags);
          ImGui::ColorPicker3(("Specular##pl" + std::to_string(index)).c_str(), glm::value_ptr(pointLight.colors.specular), lightColorEditFlags);

          ImGui::Unindent();
        }
        ++index;
      }

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Spotlights")) {
      ImGui::Indent();

      size_t index = 0;
      for (Spotlight& spotlight : std::ranges::views::values(state.scene->spotlights)) {
        if (ImGui::CollapsingHeader(std::format("{}##sl{}", spotlight.name, index).c_str())) {
          ImGui::Indent();

          ImGui::DragFloat3(("Position##sl" + std::to_string(index)).c_str(), glm::value_ptr(spotlight.position), 0.001f, -1.0f, 1.0f);
          ImGui::DragFloat3(("Direction##sl" + std::to_string(index)).c_str(), glm::value_ptr(spotlight.direction), 0.001f, -1.0f, 1.0f);
          ImGui::Spacing();

          ImGui::DragFloat(("Cut off##sl" + std::to_string(index)).c_str(), &spotlight.cutOff, 0.01f, 1.0f, spotlight.outerCutOff);
          ImGui::DragFloat(("Outer cut off##sl" + std::to_string(index)).c_str(), &spotlight.outerCutOff, 0.01f, spotlight.cutOff, 120.0f);
          ImGui::Spacing();

          ImGui::ColorPicker3(("Ambient##sl" + std::to_string(index)).c_str(), glm::value_ptr(spotlight.colors.ambient), lightColorEditFlags);
          ImGui::ColorPicker3(("Diffuse##sl" + std::to_string(index)).c_str(), glm::value_ptr(spotlight.colors.diffuse), lightColorEditFlags);
          ImGui::ColorPicker3(("Specular##sl" + std::to_string(index)).c_str(), glm::value_ptr(spotlight.colors.specular), lightColorEditFlags);

          ImGui::Unindent();
        }
        ++index;
      }

      ImGui::Unindent();
    }

    ImGui::Unindent();
  }
}

void guiActors(AppState& state) {
  if (ImGui::CollapsingHeader("Actors")) {
    ImGui::Indent();

    for (Actor& actor : std::ranges::views::values(state.scene->actors)) {
      if (ImGui::CollapsingHeader(actor.name.c_str())) {
        ImGui::Indent();

        ImGui::Text("Transform");
        Transform& grassTransform = actor.transform;
        ImGui::DragFloat3(("Translation##" + actor.name).c_str(), glm::value_ptr(grassTransform.translation), 0.01f);
        ImGui::DragFloat3(("Rotation##" + actor.name).c_str(), glm::value_ptr(grassTransform.rotation), 0.01f);
        ImGui::DragFloat3(("Scale##" + actor.name).c_str(), glm::value_ptr(grassTransform.scale), 0.01f);

        ImGui::Unindent();
      }
    }

    ImGui::Unindent();
  }
}

void guiPostProcessing(AppState& state) {
  if (ImGui::CollapsingHeader("Post processing")) {
    ImGui::Indent();

    constexpr auto firstPostProcessingEffectIndex = static_cast<int32_t>(ShaderProgramType::PostProcessCopy);
    std::optional<int32_t> effectToDelete = std::nullopt;
    for (size_t effectIndex = 0; effectIndex < state.postProcessingShaderProgramTypes.size(); effectIndex++) {
      const size_t fsTypeNameIndex = static_cast<size_t>(state.postProcessingShaderProgramTypes[effectIndex]) - firstPostProcessingEffectIndex;
      ShaderProgramType& shaderProgramType = state.postProcessingShaderProgramTypes[effectIndex];
      ShaderProgramInstanceHandle& shaderProgramInstance = state.postProcessingShaderProgramInstances[effectIndex];

      static constexpr auto fsTypeNames = std::array {
        "Copy",
        "Blur",
        "Edge detection",
        "Emboss",
        "Flip horizontally",
        "Flip vertically",
        "Grayscale",
        "Invert",
        "Sharpen",
        "Sobel bottom",
        "Sobel left",
        "Sobel right",
        "Sobel top",
      };

      const std::string effectIndexStr = std::to_string(effectIndex);

      if (ImGui::BeginCombo(("##postProcessingEffect" + effectIndexStr).c_str(), fsTypeNames[fsTypeNameIndex])) {
        for (size_t fsTypeOptionIndex = 0; fsTypeOptionIndex < fsTypeNames.size(); fsTypeOptionIndex++) {
          if (ImGui::MenuItem(fsTypeNames[fsTypeOptionIndex])) {
            shaderProgramType = static_cast<ShaderProgramType>(fsTypeOptionIndex + firstPostProcessingEffectIndex);
            shaderProgramInstance.erase();
            shaderProgramInstance = state.renderingEngine->createShaderProgramInstance(shaderProgramType);
          }
        }
        ImGui::EndCombo();
      }

      ImGui::SameLine();
      if (ImGui::Button(("Delete##postProcessingEffect" + effectIndexStr).c_str())) {
        effectToDelete = static_cast<int32_t>(effectIndex);
      }

      if (shaderProgramInstance->uniforms.contains("uOffset")) {
        auto* uOffset = shaderProgramInstance->uniforms.at("uOffset").getPtr<GLfloat>();
        ImGui::DragFloat(("Offset##postProcessingEffect" + effectIndexStr).c_str(), uOffset, 0.00001f, 0.0f, 0.01f, "%f.04");
      }
    }

    if (effectToDelete.has_value()) {
      state.postProcessingShaderProgramTypes.erase(state.postProcessingShaderProgramTypes.begin() + effectToDelete.value());
      state.postProcessingShaderProgramInstances[static_cast<size_t>(effectToDelete.value())].erase();
      state.postProcessingShaderProgramInstances.erase(state.postProcessingShaderProgramInstances.begin() + effectToDelete.value());
      state.postProcessingFramebuffers.erase(state.postProcessingFramebuffers.begin() + effectToDelete.value());
    }

    if (ImGui::Button("+ Add effect")) {
      state.postProcessingShaderProgramTypes.push_back(ShaderProgramType::PostProcessCopy);
      state.postProcessingShaderProgramInstances.push_back(state.renderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessCopy));

      const FramebufferHandle newFramebuffer = state.renderingEngine->addFramebuffer({
        state.windowSize,
        GL_RGB,
        false,
      });
      state.postProcessingFramebuffers.push_back(newFramebuffer);
    }

    ImGui::Unindent();
  }
}

void gui(AppState& state) {
  if (ImGui::Begin("Test")) {
    guiDebug(state);
    guiCamera(state);
    guiLight(state);
    guiActors(state);
    guiPostProcessing(state);
  }
  ImGui::End();
}

void initialiseImGui(GLFWwindow* window) {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

void shutdownImGui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void runImGui(AppState& state) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  gui(state);
  ImGui::Render();
}

void renderImGui() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
