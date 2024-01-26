#include <string_view>
#include <string>
#include <vector>
#include <utility>
#include <variant>

inline constexpr bool kDisableAllLogs = false;

class LogEvent final {
    friend class Logger;
public:
    LogEvent(LogEvent&&) = default;
    LogEvent(LogEvent const&) = delete;

    LogEvent& operator=(LogEvent&&) = default;
    void operator=(LogEvent const&) = delete;

    void Log(std::string_view message) && {
        if (kDisableAllLogs || !mEnabled) {
            return;
        }
        std::move(*this).LogImpl(message);
    }
    LogEvent AttrU64(std::string_view key, uint64_t value) && {
        if (!kDisableAllLogs && mEnabled) {
            AttrU64Impl(key, value);
        }
        return std::move(*this);
    }
    LogEvent AttrI64(std::string_view key, int64_t value) && {
        if (!kDisableAllLogs && mEnabled) {
            AttrI64Impl(key, value);
        }
        return std::move(*this);
    }
private:
    LogEvent() = default;

    void LogImpl(std::string_view message) &&;
    void AttrU64Impl(std::string_view key, uint64_t value);
    void AttrI64Impl(std::string_view key, int64_t value);
    bool mEnabled;
    std::string_view mPrefix;
    using Value = std::variant<uint64_t, int64_t>;

    std::vector<std::pair<std::string_view, Value>> mKvPairs;
};

class GetLoggerH;


class Logger final {
    friend class GetLoggerH;
public:
    LogEvent operator()() const;
private:
    std::string mPrefix;
    bool mEnabled;
};


Logger GetLogger(std::string_view caller);
