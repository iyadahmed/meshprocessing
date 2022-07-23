#pragma once

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((bool)(expr), true)
#define unlikely(expr) __builtin_expect((bool)(expr), false)
#endif

#ifdef NDEBUG
#define tassert(x)
#else
#define tassert(x)                                                             \
  if (!(x)) {                                                                  \
    throw;                                                                     \
  }
#endif
