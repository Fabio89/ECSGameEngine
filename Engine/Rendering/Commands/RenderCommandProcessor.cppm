export module Render.CommandProcessor;
import Log;
import Render.Commands;
import Render.RenderWorld;
import ThreadSafeQueue;
import std;

export class RenderCommandProcessor;

class RenderCommandBase
{
public:
    RenderCommandBase() = default;
    virtual ~RenderCommandBase() = default;
    RenderCommandBase(const RenderCommandBase&) = default;
    RenderCommandBase& operator=(const RenderCommandBase&) = default;
    RenderCommandBase(RenderCommandBase&&) = default;
    RenderCommandBase& operator=(RenderCommandBase&&) = default;

    virtual void process() = 0;
};

export class RenderCommandQueue
{
public:
    template<typename T>
    void addCommand(T&& command);

private:
    friend class RenderCommandProcessor;
    explicit RenderCommandQueue(RenderCommandProcessor& processor);

    RenderCommandProcessor& m_processor;
    ThreadSafeQueue<std::unique_ptr<RenderCommandBase>> m_commands;
};

struct RenderCommandProcessorContext
{
    RenderWorldManager& renderWorldManager;
};

class RenderCommandProcessor
{
public:
    explicit RenderCommandProcessor(RenderCommandProcessorContext context) : m_queue{*this}, m_context{std::move(context)} {}

    void processAll();

    template<typename T>
    void process(T&&) { fatalError("Unknown render command"); }

    RenderCommandQueue& getQueue() { return m_queue; }

private:
    RenderCommandQueue m_queue;
    RenderCommandProcessorContext m_context;
};

template <typename T>
class RenderCommand final : public RenderCommandBase
{
public:
    RenderCommand(RenderCommandProcessor& processor, T&& data) : m_processor{processor}, m_data{std::forward<T>(data)} {}

    void process() override { m_processor.process(std::forward<T>(m_data)); }

private:
    RenderCommandProcessor& m_processor;
    T m_data;
};

template<typename T>
void RenderCommandQueue::addCommand(T&& command)
{
    m_commands.push(std::make_unique<RenderCommand<T>>(m_processor, std::forward<T>(command)));
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddWorld&& cmd)
{
    m_context.renderWorldManager.registerWorld(cmd.world);
}

template<>
void RenderCommandProcessor::process(RenderCommands::RemoveWorld&& cmd)
{
    m_context.renderWorldManager.unregisterWorld(cmd.world);
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).addRenderObject(cmd.entity, std::move(cmd.mesh), std::move(cmd.texture), std::move(cmd.worldTransform), cmd.layer);
}

template<>
void RenderCommandProcessor::process(RenderCommands::RemoveObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).removeRenderObject(cmd.entity);
}

template<>
void RenderCommandProcessor::process(RenderCommands::AddLineObject&& cmd)
{
    m_context.renderWorldManager.getObjectManager(cmd.world).addLineRenderObject(cmd.entity, std::move(cmd.vertices), std::move(cmd.worldTransform));
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