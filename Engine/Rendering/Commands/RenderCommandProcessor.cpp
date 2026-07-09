module Render.CommandProcessor;

void RenderCommandProcessor::processAll()
{
    std::unique_ptr<RenderCommandBase> cmd;
    while (m_commands.tryPop(cmd))
    {
        cmd->process();
    }
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddWorld&& cmd)
{
    m_context.renderWorldManager.registerWorld(cmd.world);
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).addRenderObject(cmd.entity, cmd.mesh, cmd.texture);
}

template<>
void RenderCommandProcessor::process(RenderCommands::RemoveObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).removeRenderObject(cmd.entity);
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddLineObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).addLineRenderObject(cmd.entity, std::move(cmd.vertices));
}

template<>
void RenderCommandProcessor::process(RenderCommands::RemoveLineObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).removeLineRenderObject(cmd.entity);
}

template<>
void RenderCommandProcessor::process(RenderCommands::SetTransform&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).setObjectTransform(cmd.entity, cmd.worldTransform);
}

template<>
void RenderCommandProcessor::process(RenderCommands::SetObjectVisibility&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).setObjectVisibility(cmd.entity, cmd.visible);
}

template<>
void RenderCommandProcessor::process(RenderCommands::ClearRenderObjects&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).clear();
}

template<>
void RenderCommandProcessor::process(RenderCommands::SetCamera&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).setCamera(cmd.camera);
}