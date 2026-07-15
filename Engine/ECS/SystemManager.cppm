export module Engine.SystemManager;
export import World;
import Core;
import Engine.WorldManager;
import Render.CommandProcessor;
import Render.Viewport;
import Thread;

export struct SystemContext
{
    WorldManager& worlds;
    ViewportManager& viewports;
    RenderCommandQueue& renderCommands;
};

export struct SystemCallbacks
{
    void (*init)(SystemContext&);
    void (*update)(SystemContext&, float);
    void (*shutdown)(SystemContext&);
};

export class SystemManager
{
public:
    explicit SystemManager(const SystemContext& context) : m_context{context} {}

    void add(SystemCallbacks callbacks)
    {
        m_threadChecker.assertThread();
        if (m_initialized && callbacks.init)
            callbacks.init(m_context);
        m_systems.push_back(std::move(callbacks));
    }

    void init()
    {
        m_threadChecker.assertThread();
        for (auto& s : m_systems)
            if (s.init)
                s.init(m_context);
        m_initialized = true;
    }

    void update(float dt)
    {
        m_threadChecker.assertThread();
        for (auto& s : m_systems)
            if (s.update)
                s.update(m_context, dt);
    }

    void shutdown()
    {
        m_threadChecker.assertThread();
        for (auto& s : m_systems)
            if (s.shutdown)
                s.shutdown(m_context);
    }

private:
    std::vector<SystemCallbacks> m_systems;
    SystemContext m_context;
    ThreadOwned m_threadChecker;
    bool m_initialized{false};
};