module EditorBridge;
import Editor.Camera;
import Application;
import Component.Parent;
import Component.Transform;
import Editor.Components;
import Editor.Gizmos;
import Input;
import Math;
import Physics;

enum class EditMode : UInt8
{
    Translate = 0,
    Rotate = 1,
    Scale = 2,
    None = 3
};

namespace
{
    EditMode currentEditMode{EditMode::None};
    std::array<Entity, 3> gizmos;
    std::unordered_map<Entity, Entity> entitiesToBoundingBoxGizmos;
    Entity selected = invalidId();
    Vec2 lastCursorPosition;
}

constexpr Entity& getGizmo(EditMode editMode)
{
    return gizmos[static_cast<UInt8>(editMode)];
}

void attachToSelectedEntity(Entity gizmo)
{
    getWorld().editComponent<ParentComponent>(gizmo).parent = selected;
    getWorld().editComponent<TransformComponent>(gizmo).scale = 0.2f / getWorld().readComponent<TransformComponent>(selected).scale;
}

void setEditMode(EditMode editMode)
{
    if (editMode != currentEditMode)
    {
        currentEditMode = editMode;
        for (Entity gizmo : gizmos)
        {
            check(gizmo != invalidId(), "Gizmo entity is invalid. Please ensure all gizmos are properly initialized before setting edit mode.", ErrorType::Error);
            const bool shouldShow = selected != invalidId() && editMode != EditMode::None && gizmo == getGizmo(editMode);
            EditorUtils::setGizmoVisible(getWorld(), gizmo, shouldShow);
            if (shouldShow)
            {
                attachToSelectedEntity(gizmo);
            }
        }
    }
}

void editorInit()
{
    EditorComponents::init();
    
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

    const Vec2 cursorPosition = getCursorPosition(window);
    const Ray ray = Physics::rayFromScreenPosition(getWorld(), getPlayer(), cursorPosition);

    if (selected != invalidId() && Input::isKeyDown(KeyCode::MouseButtonLeft))
    {
        if (const Entity selectedGizmoAxis = Physics::lineTrace(getWorld(), ray, TraceChannelFlags::Gizmo); selectedGizmoAxis != invalidId())
        {
            const Vec2 dPos = cursorPosition - lastCursorPosition;
            TransformComponent& transform = getWorld().editComponent<TransformComponent>(selected);
        }
    }
    lastCursorPosition = cursorPosition;
    
    if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
    {
        const Entity previouslySelected = selected;
        selected = Physics::lineTrace(getWorld(), ray, TraceChannelFlags::Default);

        if (selected != previouslySelected)
        {
            if (previouslySelected != invalidId())
            {
                EditorUtils::setGizmoVisible(getWorld(), entitiesToBoundingBoxGizmos.at(previouslySelected), false);
            }

            if (selected != invalidId())
            {
                if (currentEditMode != EditMode::None)
                {
                    const Entity gizmo = getGizmo(currentEditMode);
                    attachToSelectedEntity(gizmo);
                    EditorUtils::setGizmoVisible(getWorld(), gizmo, true);
                }

                auto it = entitiesToBoundingBoxGizmos.find(selected);
                if (it == entitiesToBoundingBoxGizmos.end())
                    it = entitiesToBoundingBoxGizmos.emplace(selected, EditorUtils::createBoundingBoxGizmo(getWorld(), selected)).first;

                const Entity boundingBoxGizmo = it->second;
                EditorUtils::setGizmoVisible(getWorld(), boundingBoxGizmo, true);
            }
            else
            {
                if (currentEditMode != EditMode::None)
                {
                    EditorUtils::setGizmoVisible(getWorld(), getGizmo(currentEditMode), false);
                }
            }
        }
    }

    EditorCamera::setActive(window, Input::isKeyDown(KeyCode::MouseButtonRight));
    EditorCamera::update(window, getWorld(), getPlayer(), deltaTime);
}
