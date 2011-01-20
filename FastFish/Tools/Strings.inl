#include "Tools/Bits.h"

namespace FastFish {

extern const uns1_t g_mapUTF8SymbolLen[SizeUns1];

inline
uns1_t UTF8SymbolLen(char ch) throw()
{
    return g_mapUTF8SymbolLen[reinterpret_cast<uns1_t&>(ch)];
}

inline
bool UTF8LongerThan(const char* p, size_t n, size_t nSymbols) throw()
{
    for (;nSymbols; --nSymbols)
    {
        if (n < nSymbols)
            return false;
        
        size_t nSize = UTF8SymbolLen(*p);
        ffAssume(nSize && nSize <= n);
        n -= nSize;
        p += nSize;
    }
    return 0 != n;
}

inline
size_t UTF8SymbolPos(const char* pBegin, const char* pEnd, size_t nSymbol) throw()
{
    const char* p = pBegin; 
    for (;
         nSymbol && p != pEnd; 
         --nSymbol, p += UTF8SymbolLen(*p))
    {
        ffAssert(UTF8SymbolLen(*p));
    }
    return p - pBegin;
}


ffForceInline
size_t StrCSpn::operator() (const char* _psz) const throw()
{
    const uns1_t* psz = reinterpret_cast<const uns1_t*>(_psz);
    
    size_t nCount = 0;
    for (; !GetBit(m_map, *psz); ++nCount, ++psz)
    {}
    return nCount;    
}

inline
size_t StrToSizeT(const char* _psz) throw()
{
    const uns1_t* psz = reinterpret_cast<const uns1_t*>(_psz);
    
    size_t n = 0;     
    for(;;)
    {
        size_t nDig = *psz++ - '0';
        if (nDig < 10)
        {
            ffAssumeUser(n <= (MaxSizeT - nDig) / 10, "to large number");
            n = 10*n + nDig;
        }
        else
        {
            break;
        }
    }
    return n;
}

inline
void StrRewindNum(const char*& psz) throw()
{
    while('0' <= *psz && *psz <= '9')
    {
        ++psz;
    }
}

inline
size_t StrLenN(const char* pszBegin, size_t nMaxLen) throw()
{
    const char* pszEnd = pszBegin + nMaxLen;
    for (const char* psz = pszBegin; psz != pszEnd; ++psz)
    {
        if (0 == *psz)
        {
            return psz - pszBegin;
        }
    }
    
    return nMaxLen;
}


}//namespace FastFish 
