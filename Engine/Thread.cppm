export module Thread;
import std;
import Log;

export namespace Thread
{
    void registerGameThread();
    void registerRenderThread();

    void assertGameThread();
    void assertRenderThread();
}

namespace Thread
{
    std::thread::id gameThread;
    std::thread::id renderThread;
}

void Thread::registerGameThread()
{
    gameThread = std::this_thread::get_id();
}

void Thread::registerRenderThread()
{
    renderThread = std::this_thread::get_id();
}

void Thread::assertGameThread()
{
    if (std::this_thread::get_id() != gameThread)
        fatalError("Expected to run on game thread!");
}

void Thread::assertRenderThread()
{
    if (std::this_thread::get_id() != renderThread)
        fatalError("Expected to run on render thread!");
}
