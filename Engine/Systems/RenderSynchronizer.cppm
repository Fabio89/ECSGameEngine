export module System.RenderSynchronizer;
import Component.LineRender;
import Component.Model;
import Engine.SystemManager;
import EventBus;
import Math;
import Render.CommandProcessor;
import Render.Commands;
import World.Events;

import Engine;

namespace
{
    EventSubscription subscription;
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&](const WorldEvents::WorldCreated& event)
    {
        context.renderCommands.addCommand(RenderCommands::AddWorld{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::WorldDestroyed& event)
    {
         context.renderCommands.addCommand(RenderCommands::RemoveWorld{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::SceneUnloaded& event)
    {
        context.renderCommands.addCommand(RenderCommands::ClearRenderObjects{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::ComponentAdded& event)
    {
        const World& world = context.worlds.get(event.world);
        if (event.componentType == getTypeId<LineRenderComponent>())
        {
            const auto& component = world.readComponent<LineRenderComponent>(event.entity);
            context.renderCommands.addCommand(RenderCommands::AddLineObject{event.world, event.entity, component.vertices});
        }
        else if (event.componentType == getTypeId<ModelComponent>())
        {
            const auto& component = world.readComponent<ModelComponent>(event.entity);
            context.renderCommands.addCommand(RenderCommands::AddObject{event.world, event.entity, component.mesh, component.texture});
        }
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::EntityDestroyed& event)
    {
         context.renderCommands.addCommand(RenderCommands::RemoveObject{event.world, event.entity});
         context.renderCommands.addCommand(RenderCommands::RemoveLineObject{event.world, event.entity});
    });
}

void update(SystemContext& context, float deltaTime)
{
    context.worlds.forEachWorld([&](World& world)
    {
        const Entity cameraEntity = world.getActiveCamera();
        if (!cameraEntity.isValid())
            return;

        const float aspectRatio = Engine::getViewportAspectRatio();

        const TransformComponent& transform = world.readComponent<TransformComponent>(cameraEntity);
        CameraComponent& camera = world.editComponent<CameraComponent>(cameraEntity);

        camera.projectionMatrix = Math::perspective(Math::radians(camera.fov), aspectRatio, camera.nearPlane, camera.farPlane);
        camera.projectionMatrix[1][1] *= -1.0f;

        const Vec3 forward = Math::rotate(transform.rotation, forwardVector());
        camera.viewMatrix = Math::lookAt(transform.position, transform.position + forward, upVector());

        context.renderCommands.addCommand(RenderCommands::SetCamera{.world = world.getHandle(), .camera = {camera.viewMatrix, camera.projectionMatrix}});
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

export namespace RenderSynchronizer
{
    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}
