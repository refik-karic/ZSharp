#pragma once

#ifdef NDEBUG
#define ZAssert(condition)((void)0)
#else
#ifdef _MSC_VER
#define ZAssert(condition) if (!(condition)) { __debugbreak(); }
#endif
#endif
