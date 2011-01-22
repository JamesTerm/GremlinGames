#pragma once

#ifdef NDEBUG
#include <stdexcept>
#define ASSERT(cond) if (!(cond)) throw std::exception((#cond));
#define ASSERT_MSG(cond, msg) if (!(cond)) throw std::exception((msg));
#else  // #ifndef NDEBUG
#include <assert.h>
#define ASSERT(cond) assert(cond);
#define ASSERT_MSG(cond, msg) if (!(cond)){printf((msg)); assert(cond);}
#endif // #ifdef NDEBUG