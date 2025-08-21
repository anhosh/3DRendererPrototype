#include <Demos/DemoBase.hpp>

#include <AppState.hpp>
#include <Graphics/RenderPass.hpp>
#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Scene/Components/Outline.hpp>

#include <imgui.h>

#include <glm/gtx/compatibility.hpp>

#include <ranges>

Expected<void> DemoBase::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  mAssetManager = assets;
  mRenderingEngine = renderer;

  mMainSceneFramebuffer = mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .samples = 4,
    .colorAttachments = {
      ColorAttachmentInfo {
        .internalFormat = GL_RGB16, // Remove quantisation artifacts which occur during gamma correction from RGB8.
      },
    },
  });

  constexpr auto SHADOW_SIZE = glm::uvec2(4096);
  uint32_t numDirectionalLightShadowMaps = 0;
  for (const auto _ : mScene.ecs.view<CompDirectionalLight>().each()) {
    mDirectionalShadowFramebuffers.push_back(renderer->addFramebuffer({
      .size = SHADOW_SIZE,
      .samples = 1,
      .depthStencilMode = DepthStencilMode::DepthRBO,
    }));
    ++numDirectionalLightShadowMaps;
  }

  mDirectionalLightShadowMaps = renderer->createEmptyTexture2DArray();
  mDirectionalLightShadowMaps->allocate(SHADOW_SIZE, numDirectionalLightShadowMaps, GL_DEPTH_COMPONENT24);

  const ShaderProgramInstanceHandle gammaCorrectionShader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessGammaCorrection);
  mPostProcessingShaderProgramInstances.push_back(gammaCorrectionShader);
  mPostProcessingFramebuffers.push_back(mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .colorAttachments = { ColorAttachmentInfo {} },
    .depthStencilMode = DepthStencilMode::None,
  }));

  mMainCamera = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mMainCamera, "Main camera");
  mScene.ecs.emplace<CompTransform>(mMainCamera, CompTransform {
    .translation = glm::vec3(0.0f, 0.0f, 10.0f),
    .rotation = glm::vec3(-90.0f, 0.0f, 0.0f),
  });
  mScene.ecs.emplace<CompCamera>(mMainCamera);

  mScene.prepareForRendering();

  return {};
}

CommandBuffer DemoBase::render() {
  CommandBuffer commandBuffer;
  commandBuffer.commands.reserve(mDirectionalShadowFramebuffers.size() +
                                 2 + // CmdCopyShadowMapsToArrayTexture, CmdRenderPass
                                 static_cast<size_t>(mbDebugVisualiseVertexNormals)
                                 + mPostProcessingFramebuffers.size());

  for (const auto [shadowMapIndex, pack] : mScene.ecs.view<CompDirectionalLight, CompCamera>().each() | std::views::enumerate) {
    const auto [entity, light, camera] = pack;
    commandBuffer.commands.emplace_back(CmdRenderPass {
      .renderPass.dstFramebuffer = mDirectionalShadowFramebuffers[shadowMapIndex],
      .renderPass.pass = RenderScenePass {
        .scene = &mScene,
        .entityCamera = entity,
        .mode = SceneRenderMode::DepthMap,
      },
    });
  }

  commandBuffer.commands.emplace_back(CmdCopyShadowMapsToArrayTexture {
    .shadowMaps = mDirectionalShadowFramebuffers,
    .textureArray = mDirectionalLightShadowMaps,
  });

  commandBuffer.commands.emplace_back(CmdRenderPass {
    .renderPass.dstFramebuffer = mMainSceneFramebuffer,
    .renderPass.pass = RenderScenePass {
      .scene = &mScene,
      .entityCamera = mMainCamera,
      .mode = mSceneRenderMode,
      .shadowMaps.directionalShadowMaps = mDirectionalLightShadowMaps,
    }
  });

  if (mbDebugVisualiseVertexNormals) {
    commandBuffer.commands.emplace_back(CmdRenderPass {
      .renderPass.dstFramebuffer = mMainSceneFramebuffer,
      .renderPass.pass = RenderScenePass {
        .scene = &mScene,
        .entityCamera = mMainCamera,
        .mode = SceneRenderMode::VertexNormals,
        .bClearFramebuffer = false,
      },
    });
  }

  FramebufferHandle lastFramebuffer = mMainSceneFramebuffer;
  for (auto [shaderIndex, framebuffer] : mPostProcessingFramebuffers | std::views::enumerate) {
    commandBuffer.commands.emplace_back(CmdRenderPass {
      .renderPass.dstFramebuffer = framebuffer,
      .renderPass.pass = PostProcessingPass {
        .srcFramebuffer = lastFramebuffer,
        .postProcessingShader = mPostProcessingShaderProgramInstances[shaderIndex++],
      },
    });
    lastFramebuffer = framebuffer;
  }

  return commandBuffer;
}

void DemoBase::onWindowResize(GLFWwindow*, const glm::uvec2 newSize) {
  mMainSceneFramebuffer->resize(newSize);
  for (FramebufferHandle framebuffer : mPostProcessingFramebuffers) {
    framebuffer->resize(newSize);
  }
}

void DemoBase::runGUI(AppState& state) {
  ZoneScoped;

  if (ImGui::Begin("Test")) {
    this->gui(state);
  }
  ImGui::End();
}

void DemoBase::gui(AppState& state) {
  guiStats(state);
  guiDebug();
  guiActors();
  guiPostProcessing(state);
}

void DemoBase::guiStats(const AppState& state) const {
  (void)this;
  ImGui::Text("FPS: %.03f", 1.0 / (state.currentFrameTime - state.lastFrameTime));
  ImGui::Text("Frame duration: %.03f ms", (state.currentFrameTime - state.lastFrameTime) * 1000.0);
  ImGui::Text("Scene draw: %.03f ms", state.lastSceneRenderDuration * 1000.0);
  ImGui::Text("GUI draw: %.03f ms", state.lastGuiRenderDuration * 1000.0);
  ImGui::Text("Window size: %ux%u", state.windowSize.x, state.windowSize.y);
}

void DemoBase::guiDebug() {
  ZoneScoped;

  if (ImGui::CollapsingHeader("Debug")) {
    ImGui::Indent();

    static constexpr auto sceneRenderModeNames = std::array {
      "Full",
      "Wireframe",
      "Surface normal",
      "Surface depth",
    };
    ImGui::Combo("Render mode", reinterpret_cast<int32_t*>(&mSceneRenderMode),
                 sceneRenderModeNames.data(), sceneRenderModeNames.size());

    ImGui::Checkbox("Draw vertex normals", &mbDebugVisualiseVertexNormals);

    ImGui::Unindent();
  }
}

void DemoBase::guiActors() {
  ZoneScoped;

  constexpr ImGuiColorEditFlags lightColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

  if (ImGui::CollapsingHeader("Actors")) {
    ImGui::Indent();

    entt::basic_view transforms = mScene.ecs.view<const CompName>();
    for (auto [entity, name] : transforms.each()) {
      if (ImGui::CollapsingHeader(name.name.c_str())) {
        ImGui::Indent();

        if (CompTransform* transform = mScene.ecs.try_get<CompTransform>(entity)) {
          ImGui::Text("Transform");

          ImGui::DragFloat3(("Translation##" + name.name).c_str(), glm::value_ptr(transform->translation), 0.01f);
          ImGui::DragFloat3(("Rotation##" + name.name).c_str(), glm::value_ptr(transform->rotation), 0.01f);
          ImGui::DragFloat3(("Scale##" + name.name).c_str(), glm::value_ptr(transform->scale), 0.01f);

          ImGui::Separator();
        }

        if (CompCamera* camera = mScene.ecs.try_get<CompCamera>(entity)) {
          ImGui::Text("Camera");

          ImGui::DragFloat("FOV", &camera->fov, 0.1f, 10.0f, 120.0f);
          ImGui::DragFloat("Near", &camera->near, 0.01f, 0.01f, 10.0f);
          ImGui::DragFloat("Far", &camera->far, 0.01f, 10.0f, 1000.0f);
          ImGui::Checkbox("Orthographic", &camera->bOrthographic);

          ImGui::Separator();
        }

        if (CompDirectionalLight* directionalLight = mScene.ecs.try_get<CompDirectionalLight>(entity)) {
          ImGui::Text("Directional light");

          bool bChanged = false;
          bChanged |= ImGui::DragFloat3(("Direction##dl" + name.name).c_str(), glm::value_ptr(directionalLight->direction), 0.001f, -1.0f, 1.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##dl" + name.name).c_str(), glm::value_ptr(directionalLight->colors.specular), lightColorEditFlags);

          if (bChanged) {
            mScene.ecs.patch<CompDirectionalLight>(entity);
          }

          ImGui::Separator();
        }

        if (CompPointLight* pointLight = mScene.ecs.try_get<CompPointLight>(entity)) {
          ImGui::Text("Point light");

          bool bChanged = false;
          bChanged |= ImGui::DragFloat("Constant", &pointLight->constant, 0.1f, 1.0f, 100.0f);
          bChanged |= ImGui::DragFloat("Linear", &pointLight->linear, 0.01f, 0.01f, 10.0f);
          bChanged |= ImGui::DragFloat("Quadratic", &pointLight->quadratic, 0.001f, 0.01f, 1.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##pl" + name.name).c_str(), glm::value_ptr(pointLight->colors.specular), lightColorEditFlags);

          if (bChanged) {
            mScene.ecs.patch<CompPointLight>(entity);
          }

          ImGui::Separator();
        }

        if (CompSpotlight* spotlight = mScene.ecs.try_get<CompSpotlight>(entity)) {
          ImGui::Text("Spotlight");

          bool bChanged = false;
          bChanged |= ImGui::DragFloat3(("Direction##sl" + name.name).c_str(), glm::value_ptr(spotlight->direction), 0.001f, -1.0f, 1.0f);
          bChanged |= ImGui::DragFloat(("Cut off##sl" + name.name).c_str(), &spotlight->cutOff, 0.01f, 1.0f, spotlight->outerCutOff);
          bChanged |= ImGui::DragFloat(("Outer cut off##sl" + name.name).c_str(), &spotlight->outerCutOff, 0.01f, spotlight->cutOff, 120.0f);
          bChanged |= ImGui::ColorPicker3(("Ambient##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.ambient), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Diffuse##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.diffuse), lightColorEditFlags);
          bChanged |= ImGui::ColorPicker3(("Specular##sl" + name.name).c_str(), glm::value_ptr(spotlight->colors.specular), lightColorEditFlags);

          if (bChanged) {
            mScene.ecs.patch<CompSpotlight>(entity);
          }

          ImGui::Separator();
        }

        if (mScene.ecs.all_of<CompGraphics>(entity)) {
          ImGui::Text("Outline");
          bool bOutlined = mScene.ecs.all_of<CompOutline>(entity);
          if (ImGui::Checkbox(("Draw outline##" + name.name).c_str(), &bOutlined)) {
            if (bOutlined) {
              const ShaderProgramInstanceHandle outlineShader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::Outline);
              mScene.ecs.emplace<CompOutline>(entity, outlineShader);
            } else {
              mScene.ecs.erase<CompOutline>(entity);
            }
          }

          if (CompOutline* outline = mScene.ecs.try_get<CompOutline>(entity)) {
            auto& outlineColor = outline->outlineShader->uniforms["uOutlineColor"].getRef<glm::vec3>();
            ImGui::ColorPicker3(("Outline color##" + name.name).c_str(), glm::value_ptr(outlineColor));
          }
          ImGui::Separator();
        }

        ImGui::Unindent();
      }
    }

    ImGui::Unindent();
  }
}

void DemoBase::guiPostProcessing(const AppState& state) {
  ZoneScoped;

  if (ImGui::CollapsingHeader("Post processing")) {
    ImGui::Indent();

    constexpr auto firstPostProcessingEffectIndex = static_cast<int32_t>(ShaderProgramType::PostProcessCopy);
    std::optional<int32_t> effectToDelete = std::nullopt;
    for (auto [effectIndex, effectShader] : mPostProcessingShaderProgramInstances | std::views::enumerate) {
      const size_t fsTypeNameIndex = static_cast<size_t>(effectShader->type()) - firstPostProcessingEffectIndex;
      ShaderProgramInstanceHandle& shaderProgramInstance = effectShader;

      static constexpr auto fsTypeNames = std::array {
        "Copy",
        "Blur",
        "Edge detection",
        "Emboss",
        "Flip horizontally",
        "Flip vertically",
        "Gamma correction",
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
        for (auto [fsTypeOptionIndex, fsTypeName] : fsTypeNames | std::views::enumerate) {
          if (ImGui::MenuItem(fsTypeName)) {
            const auto shaderProgramType = static_cast<ShaderProgramType>(fsTypeOptionIndex + firstPostProcessingEffectIndex);
            shaderProgramInstance.erase();
            shaderProgramInstance = mRenderingEngine->createShaderProgramInstance(shaderProgramType);
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
        ImGui::DragFloat(("Offset##postProcessingEffect" + effectIndexStr).c_str(), uOffset, 0.00001f, 0.0f, 0.01f, "%.05f");
      }

      if (shaderProgramInstance->uniforms.contains("uGamma")) {
        auto* uOffset = shaderProgramInstance->uniforms.at("uGamma").getPtr<GLfloat>();
        ImGui::DragFloat(("Gamma##postProcessingEffect" + effectIndexStr).c_str(), uOffset, 0.001f, 1.0f, 10.0f, "%.03f");
      }
    }

    if (effectToDelete.has_value()) {
      mPostProcessingShaderProgramInstances[static_cast<size_t>(effectToDelete.value())].erase();
      mPostProcessingShaderProgramInstances.erase(mPostProcessingShaderProgramInstances.begin() + effectToDelete.value());
      mPostProcessingFramebuffers.erase(mPostProcessingFramebuffers.begin() + effectToDelete.value());
    }

    if (ImGui::Button("+ Add effect")) {
      const ShaderProgramInstanceHandle shader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessCopy);
      const FramebufferHandle newFramebuffer = mRenderingEngine->addFramebuffer({
        .size = state.windowSize,
        .colorAttachments = { ColorAttachmentInfo {} },
        .depthStencilMode = DepthStencilMode::None,
      });

      mPostProcessingShaderProgramInstances.push_back(shader);
      mPostProcessingFramebuffers.push_back(newFramebuffer);
    }

    ImGui::Unindent();
  }
}
