export module UI.Widget.EntityExplorer;
import Core;
import Component.Hierarchy;
import Component.Name;
import Component.Model;
import Component.Transform;
import ComponentRegistry;
import Math;
import Properties;
import UI.ImGui;
import UI.Widget;
import World;

export namespace Widgets
{
    class ImGuiDemo : public Widget
    {
    public:
        using Widget::Widget;

    private:
        void doDraw(World&) override 
        {
            if (m_showDemoWindow)
                ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        bool m_showDemoWindow{true};
    };

    class EntityExplorer : public Widget
    {
    public:
        using Widget::Widget;

    private:
        void doDraw(World& world) override
        {
            static float splitRatio = 0.5f; // The initial ratio of the split
            static float splitterSize = 5.0f; // The size of the splitter

            ImGui::Begin("Main Window", &m_open);

            ImGui::ImVec2 contentRegion = ImGui::GetContentRegionAvail();

            // First child window
            ImGui::BeginChild("Hierarchy", ImGui::ImVec2(contentRegion.x * splitRatio, contentRegion.y), ImGui::ImGuiChildFlags_Borders);
            ImGui::Text("Hierarchy");


            for (Entity entity : world.getEntitiesRange())
            {
                if (HierarchyUtils::getParent(world, entity) == invalidId())
                    addEntity(world, entity);
            }

            // Add more widgets to the left section here
            ImGui::EndChild();
            // Push style variables to set no padding around the invisible button
            ImGui::PushStyleVar(ImGui::ImGuiStyleVar_ItemSpacing, ImGui::ImVec2(0, 0));
            ImGui::PushStyleVar(ImGui::ImGuiStyleVar_ItemInnerSpacing, ImGui::ImVec2(0, 0));

            // Same line to place the splitter next to the left child window
            ImGui::SameLine();

            // Splitter
            ImGui::InvisibleButton("splitter", ImGui::ImVec2(splitterSize, contentRegion.y));
            if (ImGui::IsItemActive())
            {
                splitRatio += ImGui::GetIO().MouseDelta.x / contentRegion.x;
                splitRatio = std::clamp(splitRatio, 0.1f, 0.9f);
            }

            // Change cursor to indicate resizing
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGui::ImGuiMouseCursor_ResizeEW);

            ImGui::SameLine();

            // Pop style variables to revert to the previous padding
            ImGui::PopStyleVar(2);

            // Second child window
            ImGui::BeginChild("Details", ImGui::ImVec2(0, 0), ImGui::ImGuiChildFlags_Borders);
            ImGui::Text("Details");

            if (m_selectedEntity != invalidId())
            {
                for (const ComponentTypeId typeId : world.getComponentTypesInEntity(m_selectedEntity))
                {
                    if (typeId != Component<NameComponent>::typeId())
                    {
                        addComponent(world, m_selectedEntity, typeId);
                    }
                }

            }

            ImGui::EndChild();

            ImGui::End();
        }

        bool isSelected(Entity entity, ComponentTypeId componentType) const
        {
            return m_currentlySelected.has_value() && m_currentlySelected->first == entity && m_currentlySelected->second == componentType;
        }

        void addEntity(const World& world, Entity entity)
        {
            auto nameComponent = world.readComponent<NameComponent>(entity);

            ImGui::PushID(entity);

            ImGui::ImGuiTreeNodeFlags flags =
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;

            if (entity == m_selectedEntity)
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
            }

            if (!HierarchyUtils::hasChildren(world, entity))
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;
            }

            bool opened = ImGui::TreeNodeEx(nameComponent.name.c_str(), flags);

            // Detect click on the row
            if (ImGui::IsItemClicked() | ImGui::IsItemFocused())
            {
                m_selectedEntity = entity;
            }

            if (opened)
            {
                for(Entity child : HierarchyUtils::children(world, entity))
                {
                    addEntity(world, child);
                }

                ImGui::TreePop(); // Close Parent Node
            }

            ImGui::PopID();
        }

        void addComponent(World& world, Entity entity, TypeId componentTypeId)
        {
            ImGui::Separator();

            ImGui::ImGuiTreeNodeFlags flags =
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;

            if (m_currentlySelected.has_value() && entity == m_currentlySelected->first && componentTypeId == m_currentlySelected->second)
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
            }

            const ComponentTypeBase* componentType = ComponentRegistry::get(componentTypeId);
            if (!componentType->hasProperties())
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;
            }

            if (const ComponentTypeBase* type = ComponentRegistry::get(componentTypeId))
            {
                ImGui::PushID(entity);
                ImGui::PushID(componentTypeId);

                bool opened = ImGui::TreeNodeEx(type->getName().data(), flags);

                if (ImGui::IsItemClicked() | ImGui::IsItemFocused())
                {
                    m_currentlySelected = {entity, componentTypeId};
                }

                if (opened)
                {
                    for (const PropertyDescriptorBase& property : componentType->getProperties())
                    {
                        ComponentBase& component = world.editComponent(entity, componentTypeId);
                        property.draw(&component);
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
                ImGui::PopID();
            }
        }

        bool m_open{true};
        Entity m_selectedEntity{invalidId()};
        std::optional<std::pair<Entity, ComponentTypeId>> m_currentlySelected;
        bool m_transformSelected{false};
    };
}
