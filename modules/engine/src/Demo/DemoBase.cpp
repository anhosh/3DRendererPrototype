#include <Demo/DemoBase.hpp>

#include <AppState.hpp>
#include <GraphicsOpenGL/RenderPass.hpp>
#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Scene/Components/Outline.hpp>
#include <Scene/Components/Spectator.hpp>

#include <imgui.h>

#include <glm/gtx/compatibility.hpp>

#include <ranges>

Expected<void> DemoBase::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<GraphicsOpenGL::RenderingEngine>& renderer) {
  mAssetManager = assets;
  mRenderingEngine = renderer;

  // Main view
  if (mMainCamera == entt::null) {
    mMainCamera = mScene.ecs.create();
  }
  if (!mScene.ecs.all_of<CompName>(mMainCamera)) {
    mScene.ecs.emplace<CompName>(mMainCamera, "Main camera");
  }
  if (!mScene.ecs.all_of<CompCamera>(mMainCamera)) {
    mScene.ecs.emplace<CompCamera>(mMainCamera);
  }
  if (!mScene.ecs.all_of<CompSpectator>(mMainCamera)) {
    mScene.ecs.emplace<CompSpectator>(mMainCamera);
  }
  if (!mScene.ecs.all_of<CompTransform>(mMainCamera)) {
    mScene.ecs.emplace<CompTransform>(mMainCamera, CompTransform {
      .translation = glm::vec3(0.0f, 0.0f, 10.0f),
      .rotation = Rotation(0.0f, 180.0f, 0.0f),
    });
  }

  mViewFrustum = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mViewFrustum, "View frustum");
  mScene.ecs.emplace<CompTransform>(mViewFrustum);

  mMainViewColorAttachment = renderer->createEmptyTexture2D();
  mMainViewColorAttachment->allocate(glm::uvec2(1), GL_RGBA16);
  mMainViewFramebuffer = mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .samples = 4,
    .colorAttachments = {
      GraphicsOpenGL::FramebufferAttachment { .texture = &mMainViewColorAttachment.get() },
    },
  });

  // Shadow maps
  for (auto& [numShadowMaps, shadowMaps, framebuffers] : std::array {
    std::tuple(mScene.ecs.view<CompDirectionalLight>().size(), std::ref(mDirectionalLightShadowMaps), std::ref(mDirectionalLightShadowFramebuffers)),
    std::tuple(mScene.ecs.view<CompSpotlight>().size(), std::ref(mSpotlightShadowMaps), std::ref(mSpotlightShadowFramebuffers)),
  }) {
    constexpr auto shadowSize = glm::uvec2(SHADOW_MAP_SIZE);
    shadowMaps.get() = renderer->createEmptyTexture2DArray();
    shadowMaps.get()->allocate(shadowSize, glm::max(static_cast<int32_t>(numShadowMaps), 1), GL_DEPTH_COMPONENT16);
    for (uint32_t shadowMapIndex = 0; shadowMapIndex < numShadowMaps; ++shadowMapIndex) {
      framebuffers.get().push_back(renderer->addFramebuffer(GraphicsOpenGL::FramebufferCreateInfo {
        .size = shadowSize,
        .samples = 1,
        .depthStencilMode = GraphicsOpenGL::DepthStencilMode::DepthAttachment,
        .depthStencilAttachment = GraphicsOpenGL::FramebufferAttachment {
          .texture = &shadowMaps.get().get(),
          .layer = shadowMapIndex,
        },
      }));
    }
  }

  // Post-processing effects
  mPostProcessingColorAttachments.emplace_back(mRenderingEngine->createEmptyTexture2D());
  mPostProcessingColorAttachments.back()->allocate(glm::uvec2(1), GL_RGBA8);

  const GraphicsOpenGL::ShaderProgramInstanceHandle gammaCorrectionShader =
    mRenderingEngine->createShaderProgramInstance(GraphicsOpenGL::ShaderProgramType::PostProcessGammaCorrection);
  mPostProcessingShaderProgramInstances.push_back(gammaCorrectionShader);

  mPostProcessingFramebuffers.push_back(mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .depthStencilMode = GraphicsOpenGL::DepthStencilMode::None,
    .colorAttachments = {
      GraphicsOpenGL::FramebufferAttachment { .texture = &mPostProcessingColorAttachments.back().get() },
    },
  }));

  mScene.prepareForRendering();

  return {};
}

void DemoBase::update(double dt [[maybe_unused]]) {
  ZoneScopedN("DemoBase::update");

  if (mbViewFrustumFollowsMainView) {
    mScene.ecs.patch<CompTransform>(mViewFrustum, [&](CompTransform& frustumTransform) {
      frustumTransform = mScene.ecs.get<CompTransform>(mMainCamera);
    });
  }
}

GraphicsOpenGL::CommandBuffer DemoBase::render() {
  GraphicsOpenGL::CommandBuffer commandBuffer;
  commandBuffer.commands.reserve(mDirectionalLightShadowFramebuffers.size() +
                                 mSpotlightShadowFramebuffers.size() +
                                 1 + // CmdRenderPass
                                 mPostProcessingFramebuffers.size() +
                                 static_cast<size_t>(mbDebugVisualiseVertexNormals) +
                                 static_cast<size_t>(mbDrawDirectionalLightsViewFrustums) +
                                 static_cast<size_t>(mbDrawSceneBoundingBoxes) +
                                 static_cast<size_t>(mbDrawViewFrustum));

  for (const auto [shadowMapIndex, pack] : mScene.ecs.view<CompDirectionalLight, CompCamera>().each() | std::views::enumerate) {
    const auto [entity, light, camera] = pack;
    commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdRenderPass {
      .renderPass = GraphicsOpenGL::RenderPass {
        .dstFramebuffer = mDirectionalLightShadowFramebuffers[shadowMapIndex],
        .pass = GraphicsOpenGL::RenderPassScene {
          .scene = &mScene,
          .entityCamera = entity,
          .mode = GraphicsOpenGL::SceneRenderMode::DepthMap,
        },
      },
    });
  }

  for (const auto [shadowMapIndex, pack] : mScene.ecs.view<CompSpotlight, CompCamera>().each() | std::views::enumerate) {
    const auto [entity, light, camera] = pack;
    commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdRenderPass {
      .renderPass = GraphicsOpenGL::RenderPass {
        .dstFramebuffer = mSpotlightShadowFramebuffers[shadowMapIndex],
        .pass = GraphicsOpenGL::RenderPassScene {
          .scene = &mScene,
          .entityCamera = entity,
          .mode = GraphicsOpenGL::SceneRenderMode::LinearizedDepthMap,
        },
      },
    });
  }

  commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdRenderPass {
    .renderPass = GraphicsOpenGL::RenderPass {
      .dstFramebuffer = mMainViewFramebuffer,
      .pass = GraphicsOpenGL::RenderPassScene {
        .scene = &mScene,
        .entityCamera = mMainCamera,
        .mode = mSceneRenderMode,
        .shadowMaps = GraphicsOpenGL::ShadowMaps {
          .directionalShadowMaps = mDirectionalLightShadowMaps,
          .spotlightShadowMaps = mSpotlightShadowMaps,
        },
      },
    }
  });

  GraphicsOpenGL::FramebufferHandle lastFramebuffer = mMainViewFramebuffer;
  for (auto [shaderIndex, framebuffer] : mPostProcessingFramebuffers | std::views::enumerate) {
    commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdRenderPass {
      .renderPass = GraphicsOpenGL::RenderPass {
        .dstFramebuffer = framebuffer,
        .pass = GraphicsOpenGL::PostProcessingPass {
          .srcFramebuffer = lastFramebuffer,
          .postProcessingShader = mPostProcessingShaderProgramInstances[shaderIndex++],
        },
      },
    });
    lastFramebuffer = framebuffer;
  }

  if (mbDebugVisualiseVertexNormals) {
    commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdRenderPass {
      .renderPass = GraphicsOpenGL::RenderPass {
        .dstFramebuffer = mMainViewFramebuffer,
        .pass = GraphicsOpenGL::RenderPassScene {
          .scene = &mScene,
          .entityCamera = mMainCamera,
          .mode = GraphicsOpenGL::SceneRenderMode::VertexNormals,
          .bClearFramebuffer = false,
        },
      },
    });
  }

  if (mbDrawSceneBoundingBoxes) {
    constexpr auto boundingBoxColor = glm::vec3(1.0f, 1.0f, 0.0f);
    const auto cmdDrawSceneBounds = GraphicsOpenGL::CmdDrawDebugFrustum(Frustum::fromAABB(mScene.bounds()), boundingBoxColor);
    commandBuffer.commands.emplace_back(cmdDrawSceneBounds);
    for (const auto [enttGraphics, graphics, transform] : mScene.ecs.view<const CompGraphics, const CompTransform>().each()) {
      for (const GraphicsOpenGL::RenderData& renderData : graphics.renderData) {
        const Frustum frustum = Frustum::fromAABB(renderData.mesh->boundingBox().transformed(transform.modelMatrix()));
        commandBuffer.commands.emplace_back(GraphicsOpenGL::CmdDrawDebugFrustum(frustum, boundingBoxColor));
      }
    }
  }

  if (mbDrawViewFrustum) {
    const CompCamera& mainCamera = mScene.ecs.get<const CompCamera>(mMainCamera);
    const CompTransform& frustumTransform = mScene.ecs.get<const CompTransform>(mViewFrustum);
    const auto command = GraphicsOpenGL::CmdDrawDebugFrustum(mainCamera.viewFrustumPerspective(frustumTransform), glm::vec3(0.0f, 1.0f, 0.0f));
    commandBuffer.commands.emplace_back(command);
  }

  if (mbDrawDirectionalLightsViewFrustums) {
    for (const auto [enttLight, lightTransform, light, lightCamera] : mScene.ecs.view<const CompTransform,
                                                                                      const CompDirectionalLight,
                                                                                      const CompCamera>().each())
    {
      const auto command = GraphicsOpenGL::CmdDrawDebugFrustum(lightCamera.viewFrustumOrthographic(lightTransform), glm::vec3(1.0f, 0.0f, 0.0f));
      commandBuffer.commands.emplace_back(command);
    }
  }

  return commandBuffer;
}

void DemoBase::onWindowResize(GLFWwindow*, const glm::uvec2 newSize) {
  mWindowSize = newSize;

  auto& [clipBox, fov] = mScene.ecs.get<CompCamera>(mMainCamera);
  clipBox.min.x = 0.0f;
  clipBox.min.y = 0.0f;
  clipBox.max.x = static_cast<float>(newSize.x);
  clipBox.max.y = static_cast<float>(newSize.y);

  mMainViewColorAttachment->destroy();
  mMainViewColorAttachment->init();
  mMainViewColorAttachment->allocate(newSize, GL_RGBA16);

  for (GraphicsOpenGL::Texture2DHandle texture : mPostProcessingColorAttachments) {
    texture->destroy();
    texture->init();
    texture->allocate(newSize, GL_RGBA8);
  }

  mMainViewFramebuffer->resize(newSize);
  for (GraphicsOpenGL::FramebufferHandle framebuffer : mPostProcessingFramebuffers) {
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
    ImGui::Checkbox("Draw scene bounding boxes", &mbDrawSceneBoundingBoxes);
    ImGui::Checkbox("Draw directional light view frusta", &mbDrawDirectionalLightsViewFrustums);
    ImGui::Checkbox("Draw view frustum", &mbDrawViewFrustum);
    ImGui::Checkbox("View frustum follows camera", &mbViewFrustumFollowsMainView);
    ImGui::Checkbox("Directional lights follow camera", &mScene.ecs.get<CompSpectator>(mMainCamera).bDirectionalLightsFollowSpectator);

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

          bool bChanged = false;
          bChanged |= ImGui::DragFloat3(("Translation##" + name.name).c_str(), glm::value_ptr(transform->translation), 0.01f);
          bChanged |= ImGui::DragFloat3(("Rotation##" + name.name).c_str(), transform->rotation.asFloatPtr(), 0.01f);
          bChanged |= ImGui::DragFloat3(("Scale##" + name.name).c_str(), glm::value_ptr(transform->scale), 0.01f);

          if (bChanged) {
            mScene.ecs.patch<CompTransform>(entity);
          }

          ImGui::Separator();
        }

        if (CompCamera* camera = mScene.ecs.try_get<CompCamera>(entity)) {
          ImGui::Text("Camera");

          bool bChanged = false;
          bChanged |= ImGui::DragFloat("FOV", &camera->fov, 0.1f, 10.0f, 120.0f);
          bChanged |= ImGui::DragFloat("Near", &camera->clipBox.min.z, 0.01f, 0.01f, 10.0f);
          bChanged |= ImGui::DragFloat("Far", &camera->clipBox.max.z, 0.01f, 10.0f, 1000.0f);

          if (bChanged) {
            mScene.ecs.patch<CompCamera>(entity);
          }

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
              const GraphicsOpenGL::ShaderProgramInstanceHandle outlineShader = mRenderingEngine->createShaderProgramInstance(GraphicsOpenGL::ShaderProgramType::Outline);
              mScene.ecs.emplace<CompOutline>(entity, outlineShader);
            } else {
              mScene.ecs.erase<CompOutline>(entity);
            }
          }

          if (CompOutline* outline = mScene.ecs.try_get<CompOutline>(entity)) {
            auto& outlineColor = outline->outlineShader->uniforms["uColor"].getRef<glm::vec3>();
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

    constexpr auto firstPostProcessingEffectIndex = static_cast<int32_t>(GraphicsOpenGL::ShaderProgramType::PostProcessCopy);
    std::optional<int32_t> effectToDelete = std::nullopt;
    for (auto [effectIndex, effectShader] : mPostProcessingShaderProgramInstances | std::views::enumerate) {
      const size_t fsTypeNameIndex = static_cast<size_t>(effectShader->type()) - firstPostProcessingEffectIndex;
      GraphicsOpenGL::ShaderProgramInstanceHandle& shaderProgramInstance = effectShader;

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
            const auto shaderProgramType = static_cast<GraphicsOpenGL::ShaderProgramType>(fsTypeOptionIndex + firstPostProcessingEffectIndex);
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
      mPostProcessingColorAttachments.emplace_back(mRenderingEngine->createEmptyTexture2D());
      mPostProcessingColorAttachments.back()->allocate(state.windowSize, GL_RGBA8);
      const GraphicsOpenGL::ShaderProgramInstanceHandle shader = mRenderingEngine->createShaderProgramInstance(GraphicsOpenGL::ShaderProgramType::PostProcessCopy);
      const GraphicsOpenGL::FramebufferHandle newFramebuffer = mRenderingEngine->addFramebuffer({
        .size = state.windowSize,
        .depthStencilMode = GraphicsOpenGL::DepthStencilMode::None,
        .colorAttachments = {
          GraphicsOpenGL::FramebufferAttachment { .texture = &mPostProcessingColorAttachments.back().get() },
        },
      });

      mPostProcessingShaderProgramInstances.push_back(shader);
      mPostProcessingFramebuffers.push_back(newFramebuffer);
    }

    ImGui::Unindent();
  }
}
