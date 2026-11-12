// ----------------------------------------------------------------------- //
//
// MODULE  : CaselessString.h
//
// PURPOSE : A string that compares without case
//
// RETAIL: Tron 2.0's: the string of its commands (AutoCommand.h) and of
// other names (CBossButeMgr's socket and node) is not an std::string: it
// has copies of the string code of its own (retail Object.lto 0x10012480,
// 0x10012830, 0x10015150), a length that allows NULL (0x10015130) and
// compares with _memicmp (0x10012430). The names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __CASELESSSTRING_H__
#define __CASELESSSTRING_H__

#include <string>

// RETAIL: the length is never expanded inline (Object.lto 0x10015130 and
// CShell.dll 0x10088c30, called everywhere, one copy per image) while
// compare is: so the length is a file-static helper that is not marked
// inline (/Ob1 does not expand it; /OPT:ICF folds the copies).  Ours.
static size_t CaselessStrLen(const char* s)
{
	if (!s) return 0;
	return strlen(s);
}

// A char_traits that compares without case (the name is ours)
struct CaselessCharTraits : public std::char_traits<char>
{
	static int compare(const char* s1, const char* s2, size_t n)
	{
		return _memicmp(s1, s2, n);
	}

	static size_t length(const char* s)
	{
		return CaselessStrLen(s);
	}
};

typedef std::basic_string<char, CaselessCharTraits> CaselessString;

#endif // __CASELESSSTRING_H__
