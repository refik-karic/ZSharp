#pragma once

#ifdef NDEBUG
#define ZAssert(condition)((void)0)
#else
#ifdef PLATFORM_WINDOWS
#define ZAssert(condition) if (!(condition)) { __debugbreak(); }
#endif
#endif
