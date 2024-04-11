#pragma once
namespace detail {
inline constexpr bool kDisableAllLogs =
#if LOGGER_DISABLE_ALL_LOGGING == 1
true
#else
false
#endif
;
}
