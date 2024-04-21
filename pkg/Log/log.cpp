#include "log/log.h"

#include "filter.h"

#include <iostream>
#include <mutex>
#include <string>
//#include <set>

using namespace std::string_literals;
namespace logger {
namespace {
std::mutex gLoggingLock;
}

void LogEvent::AttrU64Impl(std::string_view key, uint64_t value) {
    mKvPairs.emplace_back(key, value);
}
void LogEvent::AttrI64Impl(std::string_view key, int64_t value) {
    mKvPairs.emplace_back(key, value);
}
void LogEvent::AttrSImpl(std::string_view key, std::string_view value) {
    mKvPairs.emplace_back(key, value);
}


void LogEvent::LogImpl(std::string_view message) && {
    std::lock_guard g(gLoggingLock);
    std::cout << mPrefix;
    std::cout << message;
    for (auto const& p : mKvPairs) {
        std::cout << ' ';
        std::cout << p.first << '=';
        std::visit([](auto x) {
            std::cout << x;
        }, p.second);
    }
    std::cout << '\n';
}

LogEvent Logger::operator()() const {
    LogEvent b;
    b.mEnabled = mEnabled;
    if (mEnabled) {
        b.mPrefix = mPrefix;
    }
    return b;
}

namespace {
struct LoggingConfig {
    std::unique_ptr<logger::Filter> filter;

};


std::string GetEnv(std::string const& name) {
    char* s = std::getenv(name.c_str());
    if (s == nullptr) {
        return {};
    }
    return {s};
}

LoggingConfig LoadConfig() {
    LoggingConfig c;
    c.filter = Filter::Parse(GetEnv("LOG_DISABLE"));
    return c;
}

}


class GetLoggerH {
public:
    static Logger GetLoggerImpl(std::string caller) {
        Logger l;
        l.mPrefix = "["s + caller + "] ";
        l.mEnabled = IsEnabled(caller);
        return l;
    }
private:
    static bool IsEnabled(std::string const& caller) {
        static LoggingConfig conf = LoadConfig();
        return conf.filter->Enabled(caller);
    }
};


Logger Get(std::string_view caller) {
    return GetLoggerH::GetLoggerImpl(std::string(caller));
}
}