export module Editor.Panels.Hierarchy;
import Component.Hierarchy;
import Component.Name;
import Component.Model;
import Component.Transform;
import ComponentRegistry;
import Editor;
import Editor.ImGui;
import Editor.Panel.Impl;
import Editor.Requests;
import Engine;
import Math;
import Properties;

bool isEditorOnly(Entity entity)
{
    return Engine::hasComponent<TagsComponent>(entity) && std::ranges::contains(Engine::readComponent<TagsComponent>(entity).tags, Tag::editorOnly);
}

export namespace Panels
{
    class HierarchyPanel : public PanelImpl
    {
    public:
        using PanelImpl::PanelImpl;
        
    private:
        void doDraw() override
        {
            World& world = context().world;
            ImGui::Begin("Hierarchy", &m_open);

            for (Entity entity : world.getEntitiesRange())
            {
                if (!HierarchyUtils::getParent(world, entity).isValid())
                    drawEntity(world, entity);
            }

            ImGui::End();
        }

        void drawEntity(const World& world, Entity entity)
        {
            if (isEditorOnly(entity))
                return;

            auto nameComponent = world.readComponent<NameComponent>(entity);

            ImGui::PushID(entity.value);

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth;

            if (context().selection.contains(entity))
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            if (!HierarchyUtils::hasChildren(world, entity)
                || std::ranges::all_of(HierarchyUtils::children(world, entity), isEditorOnly))
            {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            bool opened = ImGui::TreeNodeEx(nameComponent.name.c_str(), flags);

            // Detect click on the row
            if (ImGui::IsItemClicked() || ImGui::IsItemFocused())
            {
                Editor::request(Editor::Requests::ChangeSelection{.contextId = context().id, .entities = {entity}});
            }

            if (opened)
            {
                for(Entity child : HierarchyUtils::children(world, entity))
                {
                    drawEntity(world, child);
                }

                ImGui::TreePop(); // Close Parent Node
            }

            ImGui::PopID();
        }

        bool m_open{true};
    };
}
