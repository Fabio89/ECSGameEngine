export module Render.CommandProcessor;
import Log;
import Render.Commands;
import Render.RenderWorld;
import ThreadSafeQueue;
import std;

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

struct RenderCommandProcessorContext
{
    RenderWorldManager& renderWorldManager;
};

export class RenderCommandProcessor
{
public:
    explicit RenderCommandProcessor(RenderCommandProcessorContext context) : m_context{std::move(context)} {}

    void processAll();

    template<typename T>
    void addCommand(T&& command);

    template<typename T>
    void process(T&&) { fatalError("Unknown render command"); }

private:
    ThreadSafeQueue<std::unique_ptr<RenderCommandBase>> m_commands;
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
void RenderCommandProcessor::addCommand(T&& command)
{
    m_commands.push(std::make_unique<RenderCommand<T>>(*this, std::forward<T>(command)));
}
