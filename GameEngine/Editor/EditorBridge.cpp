module EditorBridge;
import Editor.Camera;
import Application;
import Component.Transform;
import Editor.Gizmos;
import Input;
import Physics;

enum class EditMode : UInt8
{
    Translate = 0,
    Rotate = 1,
    Scale = 2,
    None = 3
};

EditMode currentEditMode{EditMode::None};

std::array<Entity, 3> gizmos;

constexpr Entity& getGizmo(EditMode editMode)
{
    return gizmos[static_cast<UInt8>(editMode)];
}

std::unordered_map<Entity, Entity> entitiesToBoundingBoxGizmos;
Entity selected = invalidId();

void setEditMode(EditMode editMode)
{
    if (editMode != currentEditMode)
    {
        currentEditMode = editMode;
        for (Entity gizmo : gizmos)
        {
            check(gizmo != invalidId(), "Gizmo entity is invalid. Please ensure all gizmos are properly initialized before setting edit mode.", ErrorType::Error);
            const bool shouldShow = selected != invalidId() && editMode != EditMode::None && gizmo == getGizmo(editMode);
            getWorld().getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmo, shouldShow});
            if (shouldShow)
                getWorld().editComponent<TransformComponent>(gizmo) = getWorld().readComponent<TransformComponent>(selected);
        }
    }
}

void editorInit()
{
    getGizmo(EditMode::Translate) = EditorUtils::createTranslationGizmo(getWorld());
    getGizmo(EditMode::Rotate) = EditorUtils::createRotationGizmo(getWorld());
    getGizmo(EditMode::Scale) = EditorUtils::createScaleGizmo(getWorld());
}

void editorUpdate(GLFWwindow* window, float deltaTime)
{
    if (Input::isKeyJustPressed(KeyCode::Q))
    {
        setEditMode(EditMode::None);
    }
    else if (Input::isKeyJustPressed(KeyCode::W))
    {
        setEditMode(EditMode::Translate);
    }
    else if (Input::isKeyJustPressed(KeyCode::E))
    {
        setEditMode(EditMode::Rotate);
    }
    else if (Input::isKeyJustPressed(KeyCode::R))
    {
        setEditMode(EditMode::Scale);
    }

    if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
    {
        Entity previouslySelected = selected;

        const Ray ray = Physics::rayFromScreenPosition(getWorld(), getPlayer(), getCursorPosition(window));
        selected = Physics::lineTrace(getWorld(), ray);

        if (selected != previouslySelected)
        {
            if (previouslySelected != invalidId())
            {
                getWorld().getRenderManager().addCommand(RenderCommands::SetObjectVisibility{entitiesToBoundingBoxGizmos.at(previouslySelected), false});
            }

            if (selected != invalidId())
            {
                if (currentEditMode != EditMode::None)
                {
                    getWorld().editComponent<TransformComponent>(getGizmo(currentEditMode)) = getWorld().readComponent<TransformComponent>(selected);
                    getWorld().getRenderManager().addCommand(RenderCommands::SetObjectVisibility{getGizmo(currentEditMode), true});
                }

                auto it = entitiesToBoundingBoxGizmos.find(selected);
                if (it == entitiesToBoundingBoxGizmos.end())
                    it = entitiesToBoundingBoxGizmos.emplace(selected, EditorUtils::createBoundingBoxGizmo(getWorld(), selected)).first;

                getWorld().getRenderManager().addCommand(RenderCommands::SetObjectVisibility{it->second, true});
            }
            else
            {
                if (currentEditMode != EditMode::None)
                {
                    getWorld().getRenderManager().addCommand(RenderCommands::SetObjectVisibility{getGizmo(currentEditMode), false});
                }
            }
        }
    }

    EditorCamera::setActive(window, Input::isKeyDown(KeyCode::MouseButtonRight));
    EditorCamera::update(window, getWorld(), getPlayer(), deltaTime);
}
