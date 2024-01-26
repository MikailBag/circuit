#include "log.h"

#include <iostream>
#include <mutex>
#include <string>
#include <set>

using namespace std::string_literals;

static std::mutex gLoggingLock;

void LogEvent::AttrU64Impl(std::string_view key, uint64_t value) {
    mKvPairs.emplace_back(key, value);
}
void LogEvent::AttrI64Impl(std::string_view key, int64_t value) {
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

static std::set<std::string> ParseDisabledLoggers(std::string_view spec) {
    std::set<std::string> ans;
    while (true) {
        size_t pos = spec.find_first_of(',');
        if (pos == std::string_view::npos) {
            if (!spec.empty()) {
                ans.emplace(spec);
            }
            break;
        }
        ans.emplace(spec.substr(0, pos));
        spec = spec.substr(pos+1);
    }
    return ans;
}

static std::string GetEnv(std::string const& name) {
    char* s = std::getenv(name.c_str());
    if (s == nullptr) {
        return {};
    }
    return {s};
}

class GetLoggerH {
public:
    static bool IsEnabled([[maybe_unused]] std::string const& caller) {
        static std::set<std::string> disabledLoggers = ParseDisabledLoggers(GetEnv("LOG_DISABLE"));
        return disabledLoggers.count(caller) == 0;
    }
    static Logger GetLoggerImpl(std::string caller) {
        Logger l;
        l.mPrefix = "["s + caller + "] ";
        l.mEnabled = IsEnabled(caller);
        return l;
    }
};


Logger GetLogger(std::string_view caller) {
    return GetLoggerH::GetLoggerImpl(std::string(caller));
}