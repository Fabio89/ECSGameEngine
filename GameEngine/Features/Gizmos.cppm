export module Feature.Gizmos;
import Component.LineRender;
import Component.Name;
import Component.Transform;
import Core;
import Math;
import Render.Model;
import World;

export namespace EditorUtils
{
    __declspec(dllexport)
    Entity createTranslationGizmo(World& world, Entity parentEntity);
}

Entity EditorUtils::createTranslationGizmo(World& world, Entity parentEntity)
{
    Entity gizmo = world.createEntity();
    world.addComponent<NameComponent>(gizmo, NameComponent{.name = "Translation Gizmo"});
    world.addComponent<TransformComponent>(gizmo, world.readComponent<TransformComponent>(parentEntity));
    world.addComponent<LineRenderComponent>
    (
        gizmo,
        LineRenderComponent
        {
            .parent = parentEntity,
            .vertices =
            {
                // X-axis line (red)
                LineVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Start of red line
                LineVertex{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // End of red line

                // Y-axis line (green)
                LineVertex{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Start of green line
                LineVertex{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // End of green line

                // Z-axis line (blue)
                LineVertex{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Start of blue line
                LineVertex{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}} // End of blue line
            }
        }
    );

    return gizmo;
}
