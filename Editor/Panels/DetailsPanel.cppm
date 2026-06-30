export module Editor.Panels.Details;
import Component.Name;
import Editor;
import ImGui;
import Editor.Panel.Impl;
import Editor.PropertyDrawers;
import Editor.Requests;
import Engine;

export namespace Panels
{
    class DetailsPanel : public PanelImpl
    {
    public:
        using PanelImpl::PanelImpl;

    private:
        void doDraw() override
        {
            World& world = context().world;
            ImGui::Begin("Details", &m_open);

            const Entity inspectedEntity = context().selection.isEmpty() ? Entity{} : context().selection.get().front();
            if (world.isValid(inspectedEntity))
            {
                for (const TypeId typeId : world.getComponentTypesInEntity(inspectedEntity))
                {
                    if (typeId != Component<NameComponent>::typeId())
                    {
                        drawComponent(context().id, inspectedEntity, typeId);
                    }
                }
            }

            ImGui::End();
        }

        void drawComponent(EditingContextId contextId, Entity entity, TypeId componentTypeId)
        {
            if (const ComponentTypeBase* componentType = ComponentRegistry::get(componentTypeId))
            {
                ImGui::Separator();

                ImGuiTreeNodeFlags flags =
                        ImGuiTreeNodeFlags_DefaultOpen |
                        ImGuiTreeNodeFlags_OpenOnArrow |
                        ImGuiTreeNodeFlags_SpanFullWidth;

                if (!componentType->hasProperties())
                {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }

                ImGui::PushID(entity.value);
                ImGui::PushID(componentTypeId.value);

                if (ImGui::TreeNodeEx(componentType->getName().data(), flags))
                {
                    const ComponentBase& component = context().world.get().readComponent(entity, componentTypeId);

                    for (const PropertyDescriptorBase& property : componentType->getProperties())
                    {
                        PropertyValue value = property.copy(&component);

                        if (Editor::drawProperty(property, value))
                        {
                            Editor::request(Editor::SetProperty{
                                .contextId = contextId,
                                .entity = entity,
                                .componentType = componentTypeId,
                                .property = &property,
                                .value = std::move(value)
                            });
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
                ImGui::PopID();
            }
        }

        bool m_open{true};
    };
}
