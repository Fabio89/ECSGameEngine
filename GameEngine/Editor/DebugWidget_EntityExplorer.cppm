export module Engine:DebugWidget.EntityExplorer;
import :ComponentRegistry;
import :Component.Name;
import :Component.Model;
import :Component.Transform;
import :DebugWidget;
import :Core;
import std;
import Wrapper.ImGui;

export namespace DebugWidgets
{
    class ImGuiDemo : public DebugWidget
    {
    public:
        using DebugWidget::DebugWidget;

    private:
        void doDraw(World&) override 
        {
            if (m_showDemoWindow)
                Wrapper_ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        bool m_showDemoWindow{true};
    };

    class EntityExplorer : public DebugWidget
    {
    public:
        using DebugWidget::DebugWidget;

    private:
        void doDraw(World& world) override
        {
            static float splitRatio = 0.5f; // The initial ratio of the split
            static float splitterSize = 5.0f; // The size of the splitter

            Wrapper_ImGui::Begin("Main Window", &m_open);

            Wrapper_ImGui::ImVec2 contentRegion = Wrapper_ImGui::GetContentRegionAvail();

            // First child window
            Wrapper_ImGui::BeginChild("Hierarchy", Wrapper_ImGui::ImVec2(contentRegion.x * splitRatio, contentRegion.y), Wrapper_ImGui::ImGuiChildFlags_Border);
            Wrapper_ImGui::Text("Hierarchy");


            for (Entity entity : world.getEntitiesRange())
            {
                auto nameComponent = world.readComponent<NameComponent>(entity);
                if (Wrapper_ImGui::TreeNodeEx(nameComponent.name.c_str(), Wrapper_ImGui::ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (ComponentTypeId typeId : world.getComponentTypesInEntity(entity))
                    {
                        if (typeId != NameComponent::typeId)
                        {
                            if (const ComponentTypeBase* type = ComponentRegistry::get(typeId))
                            {
                                if (Wrapper_ImGui::Selectable(type->getName().data(), isSelected(entity, typeId)))
                                {
                                    m_currentlySelected.emplace(entity, typeId);
                                }
                            }
                        }
                    }
                    Wrapper_ImGui::TreePop(); // Close Parent Node
                }
            }

            // Add more widgets to the left section here
            Wrapper_ImGui::EndChild();
            // Push style variables to set no padding around the invisible button
            Wrapper_ImGui::PushStyleVar(Wrapper_ImGui::ImGuiStyleVar_ItemSpacing, Wrapper_ImGui::ImVec2(0, 0));
            Wrapper_ImGui::PushStyleVar(Wrapper_ImGui::ImGuiStyleVar_ItemInnerSpacing, Wrapper_ImGui::ImVec2(0, 0));

            // Same line to place the splitter next to the left child window
            Wrapper_ImGui::SameLine();

            // Splitter
            Wrapper_ImGui::InvisibleButton("splitter", Wrapper_ImGui::ImVec2(splitterSize, contentRegion.y));
            if (Wrapper_ImGui::IsItemActive())
            {
                splitRatio += Wrapper_ImGui::GetIO().MouseDelta.x / contentRegion.x;
                splitRatio = std::clamp(splitRatio, 0.1f, 0.9f);
            }

            // Change cursor to indicate resizing
            if (Wrapper_ImGui::IsItemHovered())
                Wrapper_ImGui::SetMouseCursor(Wrapper_ImGui::ImGuiMouseCursor_ResizeEW);

            Wrapper_ImGui::SameLine();

            // Pop style variables to revert to the previous padding
            Wrapper_ImGui::PopStyleVar(2);

            // Second child window
            Wrapper_ImGui::BeginChild("Details", Wrapper_ImGui::ImVec2(0, 0), Wrapper_ImGui::ImGuiChildFlags_Border);
            Wrapper_ImGui::Text("Details");

            if (m_currentlySelected.has_value())
            {
                if (const ComponentTypeBase* componentType = ComponentRegistry::get(m_currentlySelected->second))
                {
                    if (Wrapper_ImGui::TreeNode(componentType->getName().data()))
                    {
                        // Wrapper_ImGui::BulletText(std::format("Position: X: {}; Y: {}; Z: {}", transform.position.x, transform.position.y, transform.position.z).c_str());
                        // Wrapper_ImGui::BulletText(std::format("Rotation: X: {}; Y: {}; Z: {}", transform.rotation.x, transform.rotation.y, transform.rotation.z).c_str());
                        // Wrapper_ImGui::BulletText(std::format("Scale: {}", transform.scale).c_str());

                        Wrapper_ImGui::TreePop(); // Close Child Node 1
                    }
                }
                // const ComponentTypeId componentType = m_currentlySelected->second;
                // const ComponentBase& selectedComponent = world.readComponent(m_currentlySelected->first, componentType);
                //
            }

            Wrapper_ImGui::EndChild();

            Wrapper_ImGui::End();
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
