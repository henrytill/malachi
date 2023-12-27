#include "platform.h"

// NOLINTBEGIN(bugprone-suspicious-include)
#if defined(PLATFORM_WINDOWS)
#include "config_test_windows.c"
#elif defined(PLATFORM_MACOS)
#include "config_test_macos.c"
#elif defined(PLATFORM_LINUX)
#include "config_test_linux.c"
#elif defined(PLATFORM_UNKNOWN)
#include "config_test_unknown.c"
#endif
// NOLINTEND(bugprone-suspicious-include)
