export module Thread;
import Log;
import std;

export class ThreadOwned
{
public:
    void assertThread() const
    {
        check(std::this_thread::get_id() == m_owner,
              std::format("Wrong thread.\nExpected: {}\nCurrent: {}", m_owner, std::this_thread::get_id()),
              ErrorType::FatalError);
    }

private:
    std::thread::id m_owner{std::this_thread::get_id()};
};
