module Editor.Panels.Details;
import World;

namespace Panels
{
    DetailsSnapshot buildSnapshot(const EditingContext& context);
}

void Panels::DetailsPanel::doDraw()
{
    const Snapshot* snapshot = getSnapshot();
    if (!snapshot)
        return;
    
    ImGui::Begin(Name, &m_open);

    for (const ComponentSnapshot& component : snapshot->components)
    {
        ImGui::Separator();

        ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth;

        if (!component.type->hasProperties())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        ImGui::PushID(snapshot->entity.value);
        ImGui::PushID(component.type->getTypeId().value);

        if (ImGui::TreeNodeEx(component.type->getName().data(), flags))
        {
            for (const PropertySnapshot& property : component.properties)
            {
                PropertyValue value = property.value;
                if (Editor::drawProperty(*property.descriptor, value))
                {
                    request(Requests::SetProperty{
                        .entity = snapshot->entity,
                        .componentType = component.type->getTypeId(),
                        .property = property.descriptor,
                        .value = std::move(value)
                    });
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::PopID();
    }

    ImGui::End();
}

void DetailsController::execute(Requests::SetProperty&& request)
{
    World& world = services().worlds.get(context().world);
    if (!world.isValid(request.entity))
        return;

    if (!world.hasComponent(request.entity, request.componentType))
        return;

    auto component = world.editComponent(request.entity, request.componentType);
    request.property->set(component, request.value);
}

DetailsSnapshot DetailsController::buildSnapshot(const EditingContext& context)
{
    const World& world = services().worlds.get(context.world);
    const Entity entity = context.selection.isEmpty() ? Entity{} : context.selection.get().front();

    DetailsSnapshot snapshot;

    if (world.isValid(entity))
    {
        snapshot.entity = entity;

        auto componentTypes = world.getComponentTypesInEntity(entity);
        snapshot.components.reserve(componentTypes.size());

        for (const TypeId typeId : componentTypes)
        {
            if (typeId != getTypeId<NameComponent>())
            {
                const ComponentBase& component = world.readComponent(entity, typeId);
                const ComponentTypeBase* componentType = ComponentRegistry::get(typeId);

                ComponentSnapshot& componentSnapshot = snapshot.components.emplace_back(ComponentSnapshot{.type = componentType});

                for (const PropertyDescriptorBase& property : componentType->getProperties())
                {
                    componentSnapshot.properties.push_back(PropertySnapshot{.descriptor = &property, .value = property.copy(&component)});
                }
            }
        }
    }

    return snapshot;
}

