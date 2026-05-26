#include "core_layer/debuglayer.h"
#include "core_ecs/component/transformcomponent.h"
#include "core_ecs/component/meshcomponent.h"
#include "core_resource/iresourcemanager.h"
#include "core_service/servicelocator.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

void DebugLayer::setDebugData(DebugLayerData *debugData)
{
    debugData_ = debugData;
}

bool DebugLayer::initialise()
{
    spdlog::debug("Initialising DebugLayer");
    spdlog::info("DebugLayer initialised");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return true;
}

bool DebugLayer::render()
{
    spdlog::debug("DebugLayer rendering");
    // Debug overlay window
    ImGui::Begin(
        "Debug Overlay", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
    );

    // FPS display
    ImGui::Text("FPS: %.1f", debugData_->timer.getFps());
    ImGui::Text("Frame Time: %.2f ms", debugData_->timer.getFrameTime());

    // Frame time mini graph
    const std::vector<float> &frameTimes = debugData_->timer.getFrameTimes();
    if (!frameTimes.empty())
    {
        float maxTime = 33.33f;
        ImGui::PlotLines(
            "##FrameTime", frameTimes.data(),
            static_cast<int>(frameTimes.size()), 0, nullptr, 0.0f, maxTime,
            ImVec2(0.0f, 60.0f)
        );
    }

    ImGui::Separator();

    // Iterate all entities with TransformComponent + MeshComponent
    if (debugData_->scene)
    {
        const std::vector<std::unique_ptr<Entity>> &entities =
            debugData_->scene->getEntities();
        for (const std::unique_ptr<Entity> &entity : entities)
        {
            TransformComponent *transformComp =
                entity->getComponent<TransformComponent>();
            MeshComponent *meshComp = entity->getComponent<MeshComponent>();

            if (!transformComp || !meshComp)
                continue;

            const std::string &entityName = entity->getName();

            // Sync current transform from component into our local storage
            auto it = entityTransforms_.find(entityName);
            if (it == entityTransforms_.end())
            {
                entityTransforms_[entityName] = transformComp->getTransform();
            }
            Transform &localTransform = entityTransforms_[entityName];

            // Entity header
            ImGui::Text("%s Transform", entityName.c_str());
            ImGui::Separator();

            std::string posId = "Position##" + entityName;
            std::string rotId = "Rotation##" + entityName;
            std::string scaleId = "Scale##" + entityName;

            ImGui::SliderFloat3(
                posId.c_str(), &localTransform.position.x, -5.0f, 5.0f
            );
            ImGui::SliderFloat3(
                rotId.c_str(), &localTransform.rotation.x, -180.0f, 180.0f
            );
            ImGui::SliderFloat3(
                scaleId.c_str(), &localTransform.scale.x, 0.1f, 5.0f, "%.2f"
            );

            // Write transform back to component
            transformComp->setTransform(localTransform);

            // Cube-specific: colour picker
            if (entityName == "cube")
            {
                ImGui::Separator();
                ImGui::Text("Cube Colour");
                if (ImGui::ColorPicker3(
                        "##CubeColour", &cubeColor_.x,
                        ImGuiColorEditFlags_NoOptions
                    ))
                {
                    // Update vertex colours in the cube geometry
                    IResourceManager *resourceManager =
                        ServiceLocator::instance().get<IResourceManager>();
                    if (resourceManager)
                    {
                        Geometry *cubeGeom =
                            resourceManager->getGeometry("cube");
                        if (cubeGeom && cubeGeom->vertices)
                        {
                            for (Vertex &vertex : *cubeGeom->vertices)
                            {
                                vertex.colour = cubeColor_;
                            }
                        }
                    }
                }
            }

            ImGui::Separator();
        }
    }

    ImGui::End();

    if (!renderer_->renderImGui())
        return false;

    return true;
}

void DebugLayer::shutdown()
{
    spdlog::debug("DebugLayer shutting down");
    spdlog::info("DebugLayer shut down");
}
