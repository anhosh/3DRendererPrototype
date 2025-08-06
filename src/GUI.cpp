#include <GUI.hpp>

#include <AppState.hpp>
#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Dirty.hpp>
#include <Scene/Components/Name.hpp>
#include <Scene/Components/Outline.hpp>

#include <entt/entity/registry.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ranges>

constexpr ImGuiColorEditFlags lightColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

void guiDebug(const AppState& state) {
  ZoneScoped;

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
  ZoneScoped;

  if (ImGui::CollapsingHeader("Camera")) {
    ImGui::Indent();
    CompCamera& camera = state.scene->ecs.get<CompCamera>(state.mainCamera);

    ImGui::DragFloat("Movement speed", &camera.speed, 0.001f, 0.0f, 5.0f);
    ImGui::DragFloat("FOV", &camera.fov, 0.1f, 10.0f, 120.0f);
    ImGui::DragFloat("Near", &camera.near, 0.01f, 0.01f, 10.0f);
    ImGui::DragFloat("Far", &camera.far, 0.01f, 10.0f, 1000.0f);

    ImGui::Spacing();

    ImGui::Checkbox("Back mirror", &state.bBackMirror);

    ImGui::Unindent();
  }
}

void guiActors(AppState& state) {
  ZoneScoped;

  if (ImGui::CollapsingHeader("Actors")) {
    ImGui::Indent();

    entt::basic_view transforms = state.scene->ecs.view<const CompName>();
    for (auto [entity, name] : transforms.each()) {
      if (ImGui::CollapsingHeader(name.name.c_str())) {
        ImGui::Indent();

        bool bChanged = false;

        bool bOutlined = state.scene->ecs.all_of<CompOutline>(entity);
        bChanged |= ImGui::Checkbox(("Draw outline##" + name.name).c_str(), &bOutlined);
        if (bChanged) {
          if (bOutlined) {
            const ShaderProgramInstanceHandle outlineShader = state.renderingEngine->createShaderProgramInstance(ShaderProgramType::Outline);
            state.scene->ecs.emplace<CompOutline>(entity, outlineShader);
          } else {
            state.scene->ecs.erase<CompOutline>(entity);
          }
        }

        if (CompOutline* outline = state.scene->ecs.try_get<CompOutline>(entity)) {
          ImGui::Text("Outline");

          auto& outlineColor = outline->outlineShader->uniforms["uOutlineColor"].getRef<glm::vec3>();
          bChanged |= ImGui::ColorPicker3(("Outline color##" + name.name).c_str(), glm::value_ptr(outlineColor));

          ImGui::Spacing();
        }

        if (CompTransform* transform = state.scene->ecs.try_get<CompTransform>(entity)) {
          ImGui::Text("Transform");

          bChanged |= ImGui::DragFloat3(("Translation##" + name.name).c_str(), glm::value_ptr(transform->translation), 0.01f);
          bChanged |= ImGui::DragFloat3(("Rotation##" + name.name).c_str(), glm::value_ptr(transform->rotation), 0.01f);
          bChanged |= ImGui::DragFloat3(("Scale##" + name.name).c_str(), glm::value_ptr(transform->scale), 0.01f);

          ImGui::Spacing();
        }

        if (CompDirectionalLight* directionalLight = state.scene->ecs.try_get<CompDirectionalLight>(entity)) {
          ImGui::Text("Directional light");

          bChanged |= ImGui::DragFloat3(("Direction##dl" + name.name).c_str(), glm::value_ptr(directionalLight->direction), 0.001f, -1.0f, 1.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.specular), lightColorEditFlags);

          ImGui::Spacing();
        }

        if (CompPointLight* pointLight = state.scene->ecs.try_get<CompPointLight>(entity)) {
          ImGui::Text("Point light");

          bChanged |= ImGui::DragFloat("Constant", &pointLight->constant, 0.1f, 1.0f, 100.0f);
          bChanged |= ImGui::DragFloat("Linear", &pointLight->linear, 0.01f, 0.01f, 10.0f);
          bChanged |= ImGui::DragFloat("Quadratic", &pointLight->quadratic, 0.001f, 0.01f, 1.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.specular), lightColorEditFlags);

          ImGui::Spacing();
        }

        if (CompSpotlight* spotlight = state.scene->ecs.try_get<CompSpotlight>(entity)) {
          ImGui::Text("Spotlight");

          bChanged |= ImGui::DragFloat3(("Direction##sl" + name.name).c_str(), glm::value_ptr(spotlight->direction), 0.001f, -1.0f, 1.0f);
          bChanged |= ImGui::DragFloat(("Cut off##sl" + name.name).c_str(), &spotlight->cutOff, 0.01f, 1.0f, spotlight->outerCutOff);
          bChanged |= ImGui::DragFloat(("Outer cut off##sl" + name.name).c_str(), &spotlight->outerCutOff, 0.01f, spotlight->cutOff, 120.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.specular), lightColorEditFlags);

          ImGui::Spacing();
        }

        if (bChanged) {
          state.scene->ecs.emplace_or_replace<CompDirty>(entity);
        }

        ImGui::Unindent();
      }
    }

    ImGui::Unindent();
  }
}

void guiPostProcessing(AppState& state) {
  ZoneScoped;

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
  ZoneScoped;

  if (ImGui::Begin("Test")) {
    guiDebug(state);
    guiCamera(state);
    guiActors(state);
    guiPostProcessing(state);
  }
  ImGui::End();
}

void initialiseImGui(GLFWwindow* window) {
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
  gui(state);
  ImGui::Render();
}

void renderImGui() {
  ZoneScoped;

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
