module;
#include <imgui.h>
export module Engine.DebugWidget.EntityExplorer;
import Engine.ComponentRegistry;
import Engine.Component.Name;
import Engine.Component.Model;
import Engine.Component.Transform;
import Engine.DebugWidget;
import Engine.Render.Core;
import Engine.World;
import Engine.Core;

export namespace DebugWidgets
{
    class ImGuiDemo : public DebugWidget
    {
    public:
        using DebugWidget::DebugWidget;

    private:
        void draw(World&) override
        {
            if (m_showDemoWindow)
                ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        bool m_showDemoWindow{true};
    };

    class EntityExplorer : public DebugWidget
    {
    public:
        using DebugWidget::DebugWidget;

    private:
        void draw(World& world) override
        {
            static float splitRatio = 0.5f; // The initial ratio of the split
            static float splitterSize = 5.0f; // The size of the splitter

            ImGui::Begin("Main Window", &m_open);

            ImVec2 contentRegion = ImGui::GetContentRegionAvail();

            // First child window
            ImGui::BeginChild("Hierarchy", ImVec2(contentRegion.x * splitRatio, contentRegion.y), ImGuiChildFlags_Border);
            ImGui::Text("Hierarchy");


            for (Entity entity : world.getEntitiesRange())
            {
                auto nameComponent = world.readComponent<NameComponent>(entity);
                if (ImGui::TreeNodeEx(nameComponent.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (ComponentTypeId typeId : world.getComponentTypesInEntity(entity))
                    {
                        if (typeId != NameComponent::typeId)
                        {
                            if (const ComponentTypeBase* type = ComponentRegistry::get(typeId))
                            {
                                const std::string displayName = type->getDisplayName();
                                if (ImGui::Selectable(displayName.c_str(), isSelected(entity, typeId)))
                                {
                                    m_currentlySelected.emplace(entity, typeId);
                                }
                            }
                        }
                    }
                    ImGui::TreePop(); // Close Parent Node
                }
            }

            // Add more widgets to the left section here
            ImGui::EndChild();
            // Push style variables to set no padding around the invisible button
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));

            // Same line to place the splitter next to the left child window
            ImGui::SameLine();

            // Splitter
            ImGui::InvisibleButton("splitter", ImVec2(splitterSize, contentRegion.y));
            if (ImGui::IsItemActive())
            {
                splitRatio += ImGui::GetIO().MouseDelta.x / contentRegion.x;
                if (splitRatio < 0.1f) splitRatio = 0.1f;
                if (splitRatio > 0.9f) splitRatio = 0.9f;
            }

            // Change cursor to indicate resizing
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            ImGui::SameLine();

            // Pop style variables to revert to the previous padding
            ImGui::PopStyleVar(2);

            // Second child window
            ImGui::BeginChild("Details", ImVec2(0, 0), ImGuiChildFlags_Border);
            ImGui::Text("Details");

            if (m_currentlySelected.has_value())
            {
                if (const ComponentTypeBase* componentType = ComponentRegistry::get(m_currentlySelected->second))
                {
                    const std::string displayName = componentType->getDisplayName();
                    if (ImGui::TreeNode(displayName.c_str()))
                    {
                        // ImGui::BulletText(std::format("Position: X: {}; Y: {}; Z: {}", transform.position.x, transform.position.y, transform.position.z).c_str());
                        // ImGui::BulletText(std::format("Rotation: X: {}; Y: {}; Z: {}", transform.rotation.x, transform.rotation.y, transform.rotation.z).c_str());
                        // ImGui::BulletText(std::format("Scale: {}", transform.scale).c_str());

                        ImGui::TreePop(); // Close Child Node 1
                    }
                }
                // const ComponentTypeId componentType = m_currentlySelected->second;
                // const ComponentBase& selectedComponent = world.readComponent(m_currentlySelected->first, componentType);
                //
            }

            ImGui::EndChild();

            ImGui::End();
        }
        
        bool isSelected(Entity entity, ComponentTypeId componentType) const
        {
            return m_currentlySelected.has_value() && m_currentlySelected->first == entity && m_currentlySelected->second == componentType;
        }

        bool m_open{true};
        std::optional<std::pair<Entity, ComponentTypeId>> m_currentlySelected;
        bool m_transformSelected{false};
    };
}
