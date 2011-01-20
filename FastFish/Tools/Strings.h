#pragma once
#include "Common/Common.h"

namespace FastFish {

size_t UTF8ToLower(const char* pszSrc, size_t nSrcLen, char* pszDst) throw();
size_t UTF8Check(const char* pszSrc, size_t nSrcLen) throw();
bool   UTF8LongerThan(const char* p, size_t n, size_t nSymbols) throw();
size_t UTF8SymbolPos(const char* p, size_t n, size_t nSymbol) throw();
uns1_t UTF8SymbolLen(char ch) throw();

class StrCSpn
{
public:
    StrCSpn(const char* pszDelimiters, bool bThreatCtrlAsDelimiters) throw();
    size_t operator() (const char* psz) const throw();
private:
    uns1_t m_map[SizeUns1/BitsInByte];
};

size_t  StrToSizeT(const char* psz) throw();
void    StrRewindNum(const char*& psz) throw();
size_t  StrLenN(const char* psz, size_t nMaxLen) throw();


}//namespace FastFish 

#include "Tools/Strings.inl"
