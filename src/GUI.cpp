#include <GUI.hpp>

#include <AppState.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ranges>

constexpr ImGuiColorEditFlags lightColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

void guiMetrics(const AppState& state) {
  ImGui::Text("FPS: %.03f", 1.0 / (state.currentFrameTime - state.lastFrameTime));
  ImGui::Text("Frame duration: %.03f ms", (state.currentFrameTime - state.lastFrameTime) * 1000.0);
  ImGui::Text("Scene draw: %.03f ms", state.lastSceneRenderTime * 1000.0);
  ImGui::Text("GUI draw: %.03f ms", state.lastGuiRenderTime * 1000.0);
  ImGui::Text("Window size: %ux%u", state.windowSize.x, state.windowSize.y);
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

    ImGui::Checkbox("Back mirror", &state.bBackMirror);

    ImGui::Unindent();
  }
}

void guiLight(AppState& state) {
  if (ImGui::CollapsingHeader("Light")) {
    ImGui::Indent();

    if (ImGui::CollapsingHeader("Directional light")) {
      ImGui::Indent();
      DirectionalLight& directionalLight = state.scene->directionalLight;

      ImGui::DragFloat3("Direction##dl", glm::value_ptr(directionalLight.direction), 0.001f, -1.0f, 1.0f);
      ImGui::Spacing();

      ImGui::ColorPicker3("Ambient##dl", glm::value_ptr(directionalLight.colors.ambient), lightColorEditFlags);
      ImGui::ColorPicker3("Diffuse##dl", glm::value_ptr(directionalLight.colors.diffuse), lightColorEditFlags);
      ImGui::ColorPicker3("Specular##dl", glm::value_ptr(directionalLight.colors.specular), lightColorEditFlags);

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Point light")) {
      ImGui::Indent();
      PointLight& pointLight = state.scene->pointLight;

      ImGui::DragFloat("Constant", &pointLight.constant, 0.1f, 1.0f, 100.0f);
      ImGui::DragFloat("Linear", &pointLight.linear, 0.01f, 0.01f, 10.0f);
      ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.001f, 0.01f, 1.0f);
      ImGui::Spacing();

      ImGui::ColorPicker3("Ambient##pl", glm::value_ptr(pointLight.colors.ambient), lightColorEditFlags);
      ImGui::ColorPicker3("Diffuse##pl", glm::value_ptr(pointLight.colors.diffuse), lightColorEditFlags);
      if (ImGui::ColorPicker3("Specular##pl", glm::value_ptr(pointLight.colors.specular), lightColorEditFlags)) {
        (*state.lightShader)->uniforms["uLightColor"] = pointLight.colors.specular;
      }

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Spotlight")) {
      ImGui::Indent();
      Spotlight& spotlight = state.scene->spotlight;

      ImGui::Checkbox("Follow camera", &state.bFlashlightFollowCamera);
      ImGui::DragFloat("Cut off", &spotlight.cutOff, 0.01f, 1.0f, spotlight.outerCutOff);
      ImGui::DragFloat("Outer cut off", &spotlight.outerCutOff, 0.01f, spotlight.cutOff, 120.0f);
      ImGui::Spacing();

      ImGui::ColorPicker3("Ambient##sl", glm::value_ptr(spotlight.colors.ambient), lightColorEditFlags);
      ImGui::ColorPicker3("Diffuse##sl", glm::value_ptr(spotlight.colors.diffuse), lightColorEditFlags);
      ImGui::ColorPicker3("Specular##sl", glm::value_ptr(spotlight.colors.specular), lightColorEditFlags);

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

        if (actor.name.contains("Backpack")) {
          static constexpr const char* fsTypeNames[] = {
            "Light",
            "Lit surface",
            "Outline",
            "Reflective surface",
            "Refractive surface",
            "Visualise depth",
            "Visualise normal",
          };
          if (ImGui::Combo("Fragment shader",
                           reinterpret_cast<int32_t*>(&state.backpackShaderProgramType),
                           fsTypeNames,
                           std::size(fsTypeNames)))
          {
            switch (state.backpackShaderProgramType) {
              case ShaderProgramType::Light:
                actor.setShaderProgramInstance(state.lightShader.value());
                break;
              case ShaderProgramType::LitSurface:
                actor.setShaderProgramInstance(state.litSurfaceShader.value());
                break;
              case ShaderProgramType::Outline:
                actor.setShaderProgramInstance(state.backpackOutlineShader.value());
                break;
              case ShaderProgramType::ReflectiveSurface:
                actor.setShaderProgramInstance(state.reflectiveSurfaceShader.value());
                break;
              case ShaderProgramType::RefractiveSurface:
                actor.setShaderProgramInstance(state.refractiveSurfaceShader.value());
                break;
              case ShaderProgramType::VisualiseDepth:
                actor.setShaderProgramInstance(state.visualiseDepthShader.value());
                break;
              case ShaderProgramType::VisualiseNormal:
                actor.setShaderProgramInstance(state.visualiseNormalShader.value());
                break;
              default:
                PANIC("Unexpected shader program type");
            }
          }

          if (state.backpackShaderProgramType == ShaderProgramType::RefractiveSurface) {
            auto& refractiveIndex = (*state.refractiveSurfaceShader)->uniforms["uRefractiveIndex"].getRef<GLfloat>();
            ImGui::DragFloat("Refractive index", &refractiveIndex, 0.01f, 1.0f, 10.0f);
          }

          if (ImGui::Checkbox("Draw outline##backpack", &state.bDrawBackpackOutline)) {
            actor.setOutlineShaderInstance(state.bDrawBackpackOutline ? state.backpackOutlineShader : std::nullopt);
          }

          ImGui::BeginDisabled(!state.bDrawBackpackOutline);
          std::unordered_map<std::string, ShaderUniform>& outlineUniforms = (*state.backpackOutlineShader)->uniforms;
          ImGui::ColorPicker3("Outline color##backpack", outlineUniforms["uOutlineColor"].getValuePtr<glm::vec3>(), ImGuiColorEditFlags_Float);
          ImGui::EndDisabled();
        }

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
      const int32_t fsTypeNameIndex = static_cast<int32_t>(state.postProcessingShaderProgramTypes[effectIndex]) - firstPostProcessingEffectIndex;
      ShaderProgramType& shaderProgramType = state.postProcessingShaderProgramTypes[effectIndex];
      ShaderProgramInstanceHandle& shaderProgramInstance = state.postProcessingShaderProgramInstances[effectIndex];

      static constexpr const char* fsTypeNames[] = {
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
        for (size_t fsTypeOptionIndex = 0; fsTypeOptionIndex < std::size(fsTypeNames); fsTypeOptionIndex++) {
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
    guiMetrics(state);
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
