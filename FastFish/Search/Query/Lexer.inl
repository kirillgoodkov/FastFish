#include "Tools/Strings.h"

namespace FastFish{
namespace Search{

ffForceInline
void Lexer::RewindEmpty() ffThrowAll
{
    for (;;)
    {
        switch (s_map[*m_pNext])
        {
            case tkWhiteSpace:
                ++m_pNext;
                break;            

            case tkUnknown:
                Error("invalid token");
                
            default:
                return;
        }
    }        
}

ffForceInline
void Lexer::Next() ffThrowAll
{
    if (tkNumber == s_map[*m_pNext])
    {
        StrRewindNum(m_pNext);
    }
    else
    {
        ++m_pNext;
    }
    
    RewindEmpty();
}

inline
Token Lexer::Lookup() const throw()
{
    Token tk = s_map[*m_pNext];
    if (tkNumber == tk)
    {
        m_nAttr = StrToSizeT(m_pNext);
    }
    return tk;
}

ffForceInline
bool Lexer::Match(Token tk) ffThrowAll
{
    if (Lookup() == tk)
    {
        Next();
        return true;
    }
    else
    {
        return false;
    }
}

ffForceInline
void Lexer::Expect(Token tk, const char* pszText) ffThrowAll
{
    if (tk != Lookup())                
        Error(pszText);        
    Next();        
}

ffForceInline
size_t Lexer::ExpectNum(const char* pszText) ffThrowAll
{
    if (tkNumber != Lookup())                
        Error(pszText);        
    size_t n = m_nAttr;        
    Next();        
    return n;
}


}//namespace Search
}//namespace FastFish
