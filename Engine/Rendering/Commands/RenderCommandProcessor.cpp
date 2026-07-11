module Render.CommandProcessor;

RenderCommandQueue::RenderCommandQueue(RenderCommandProcessor& processor)
    : m_processor{processor}
{
}

void RenderCommandProcessor::processAll()
{
    std::unique_ptr<RenderCommandBase> cmd;
    while (m_queue.m_commands.tryPop(cmd))
    {
        cmd->process();
    }
}
