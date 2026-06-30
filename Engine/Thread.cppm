export module Thread;
import std;
import Log;

export class ThreadOwned
{
public:
    void assertThread() const
    {
        check(std::this_thread::get_id() == m_owner, "Used on the wrong thread!");
    }

private:
    std::thread::id m_owner{std::this_thread::get_id()};
};
