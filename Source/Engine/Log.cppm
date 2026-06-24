export module Log;
import std;

constexpr auto logColorRed = "\033[31m";
constexpr auto logColorYellow = "\033[33m";
constexpr auto logColorReset = "\033[0m";

export enum class ErrorType
{
    Warning,
    Error,
    FatalError
};

export template <typename T>
void log(T&& message)
{
    std::cout << message << std::endl;
}

export template <typename T>
void print(T&& message, ErrorType type)
{
    switch (type)
    {
    case ErrorType::Warning:
        std::cerr << logColorYellow << "Warning: " << message << logColorReset << std::endl;
        break;
    case ErrorType::Error:
        std::cerr << logColorRed << "Error: " << message << logColorReset << std::endl;
        break;
    case ErrorType::FatalError:
        std::cerr << logColorRed << "Fatal Error: " << message << logColorReset << std::endl;
        break;
    }
}

export template <typename T>
bool check(bool expression, T&& message, ErrorType type = ErrorType::Error)
{
    if (!expression)
    {
        print(std::forward<T>(message), type);
        if (type == ErrorType::FatalError)
            std::abort();
        return false;
    }
    return true;
}

export template <typename T>
void report(T&& message, ErrorType type = ErrorType::Error)
{
    print(std::forward<T>(message), type);
    if (type == ErrorType::FatalError)
        std::abort();
}

export template <typename T>
void fatalError(T&& message)
{
    print(std::forward<T>(message), ErrorType::FatalError);
    std::abort();
}