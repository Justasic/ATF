#pragma once
#include <cstdlib>
#include <cstdarg>

#ifndef NDEBUG
# define dprintf(...) printf(__VA_ARGS__)
#else
# define dprintf(...)
#endif

extern char *SizeReduce(size_t size);

extern int vasprintf(char **str, const char *fmt, va_list args);

inline void *memrev(void *dest, const void *src, size_t n)
{
	// Iterators, s is beginning, e is end.
	unsigned char *s = (unsigned char*)dest, *e = ((unsigned char*)dest) + n - 1;
	// Copy to out buffer for our work
	memcpy(dest, src, n);
	// Iterate and reverse copy the bytes
	for (; s < e; ++s, --e)
	{
		unsigned char t = *s;
		*s = *e;
		*e = t;
	}
	// Return provided buffer
	return dest;
}
