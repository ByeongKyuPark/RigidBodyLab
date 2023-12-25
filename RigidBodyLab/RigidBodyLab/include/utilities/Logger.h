#include <iostream>
#include <sstream>
#include <string>

class Logger {
public:
    Logger() = default;

    template<typename... Args>
    static void Log(Args... args) {
        std::ostringstream stream;
        LogInternal(stream, args...);
        std::cout << stream.str() << '\n';
    }

private:
    template<typename T, typename... Args>
    static void LogInternal(std::ostringstream& stream, T value, Args... args) {
        stream << value;
        LogInternal(stream, args...);
    }

    // base case for the recursive variadic template function
    static void LogInternal(std::ostringstream& stream) {}
};

