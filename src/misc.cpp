#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include "flux.h"


// Pointers returns by SizeReduce should be free()'d
char *SizeReduce(size_t size)
{
	static const char *sizes[] = {
		"B",
		"KB",
		"MB",
		"GB",
		"TB",
		"PB",
		"EB",
		"ZB",
		"YB"
	};

	unsigned long sz = 0;
	for (sz = 0; size > 1024; size >>= 10, sz++)
		;

	char *str = NULL;
	asprintf(&str, "%zu %s", size, sz >= (sizeof(sizes) / sizeof(*sizes)) ? "(Hello Future!)" : sizes[sz]);
	return str;
}


int vasprintf(char **str, const char *fmt, va_list args)
{
	int size = 0;
	va_list tmpa;
	// copy
	va_copy(tmpa, args);
	// apply variadic arguments to
	// sprintf with format to get size
	size = vsnprintf(NULL, size, fmt, tmpa);
	// toss args
	va_end(tmpa);
	// return -1 to be compliant if
	// size is less than 0
	if (size < 0)
		return -1;
	// alloc with size plus 1 for `\0'
	*str = (char*)malloc(size + 1);
	// return -1 to be compliant
	// if pointer is `NULL'
	if (!*str)
		return -1;
	// format string with original
	// variadic arguments and set new size
	return vsprintf(*str, fmt, args);
}

/**
 * \fn Flux::string strip_mirc_codes(const Flux::string &str)
 * \brief Attempts to remove all mIRC color codes from an IRC message
 * \param buffer A Flux::string containing mIRC color codes
 * This function was an import of InspIRCd's mirc color stripper and was
 * rewritten into C++ for compatibility with Flux::strings
 */

// The following function was taken from m_stripcolor.cpp from InspIRCd 2.0 and
// modified to fit this project.
Flux::string StripColors(const Flux::string &input)
{
    /* refactor this completely due to SQUIT bug since the old code would strip last char and replace with \0 --peavey */
    Flux::string sentence = input;
    int seq = 0;
    Flux::string::iterator i,safei;
    for (i = sentence.begin(); i != sentence.end();)
    {
		if (*i == 3)
		    seq = 1;
		else if (seq && (( ((*i >= '0') && (*i <= '9')) || (*i == ',') ) ))
		{
		    seq++;
		    if ( (seq <= 4) && (*i == ',') )
				seq = 1;
		    else if (seq > 3)
				seq = 0;
		}
		else
		    seq = 0;

		if (seq || ((*i == 2) || (*i == 15) || (*i == 22) || (*i == 21) || (*i == 31)))
		{
		    if (i != sentence.begin())
		    {
				safei = i;
				--i;
				sentence.erase(safei);
				++i;
		    }
		    else
		    {
				sentence.erase(i);
				i = sentence.begin();
		    }
		}
		else
		    ++i;
    }

    return sentence;
}

namespace Flux
{
	Flux::string Sanitize(const Flux::string &string)
	{
		static struct special_chars
		{
			Flux::string character;
			Flux::string replace;
			special_chars(const Flux::string &c, const Flux::string &r) : character(c), replace(r) { }
		}
		special[] = {
			special_chars("  ", " "),
			special_chars("\n",""),
			special_chars("\002",""),
			special_chars("\035",""),
			special_chars("\037",""),
			special_chars("\026",""),
			special_chars("\001",""),
			special_chars("\r", ""),
			special_chars("","")
		};

		Flux::string ret = string.c_str();
		ret = StripColors(ret);
		for(int i = 0; special[i].character.empty() == false; ++i)
			ret = ret.replace_all_cs(special[i].character, special[i].replace);
		return ret.c_str();
	}
}

bool ci::ci_char_traits::eq(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] == ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
bool ci::ci_char_traits::ne(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] != ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
bool ci::ci_char_traits::lt(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] < ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
int ci::ci_char_traits::compare(const char *str1, const char *str2, size_t n)
{
	for (unsigned i = 0; i < n; ++i)
	{
		if (ascii_case_insensitive_map[static_cast<unsigned char>(*str1)] > ascii_case_insensitive_map[static_cast<unsigned char>(*str2)])
			return 1;
		if (ascii_case_insensitive_map[static_cast<unsigned char>(*str1)] < ascii_case_insensitive_map[static_cast<unsigned char>(*str2)])
			return -1;
		if (!*str1 || !*str2)
			return 0;
		++str1;
		++str2;
	}
	return 0;
}
const char *ci::ci_char_traits::find(const char *s1, int n, char c)
{
	while (n-- > 0 && ascii_case_insensitive_map[static_cast<unsigned char>(*s1)] != ascii_case_insensitive_map[static_cast<unsigned char>(c)])
		++s1;
	return n >= 0 ? s1 : NULL;
}
/** Compare two Flux::strings as ci::strings and find which one is less
 * @param s1 The first string
 * @param s2 The second string
 * @return true if s1 < s2, else false
 */
bool ci::less::operator()(const Flux::string &s1, const Flux::string &s2) const
{
	return s1.ci_str().compare(s2.ci_str()) < 0;
}
