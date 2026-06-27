export module UI.Panel.Hierarchy;
import Core;
import Component.Hierarchy;
import Component.Name;
import Component.Model;
import Component.Transform;
import ComponentRegistry;
import Engine;
import Editor;
import Math;
import Properties;
import UI.ImGui;
import UI.Panel;
import World;

bool isEditorOnly(Entity entity)
{
    return Engine::hasComponent<TagsComponent>(entity) && std::ranges::contains(Engine::readComponent<TagsComponent>(entity).tags, Tag::editorOnly);
}

export namespace Panels
{
    class HierarchyPanel : public Panel
    {
    public:
        HierarchyPanel(World& world) : Panel{world}
        {
        }

    private:
        void doDraw(World& world) override
        {
            ImGui::Begin("Hierarchy", &m_open);

            for (Entity entity : world.getEntitiesRange())
            {
                if (!HierarchyUtils::getParent(world, entity).isValid())
                    drawEntity(world, entity);
            }

            ImGui::End();
        }

        void drawEntity(World& world, Entity entity)
        {
            if (isEditorOnly(entity))
                return;

            auto nameComponent = world.readComponent<NameComponent>(entity);

            ImGui::PushID(entity.value);

            ImGui::ImGuiTreeNodeFlags flags =
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;

            if (Editor::selection().contains(entity))
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
            }

            if (!HierarchyUtils::hasChildren(world, entity)
                || std::ranges::all_of(HierarchyUtils::children(world, entity), isEditorOnly))
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;
            }

            bool opened = ImGui::TreeNodeEx(nameComponent.name.c_str(), flags);

            // Detect click on the row
            if (ImGui::IsItemClicked() | ImGui::IsItemFocused())
            {
                Editor::setSingleSelection(world, entity);
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
