#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

enum Token
{
    tkEopNot,
    tkEopAnd,
    tkEopOr,
    tkSopContainAll,
    tkSopContainSome,
    tkComma,
    tkBrRdOpen,
    tkBrRdClose,
    tkBrCrOpen,
    tkBrCrClose,
    tkNumber,
    tkEnd,
    //never returned:
    tkWhiteSpace,
    tkUnknown
};

class Lexer
{
    Lexer(const Lexer&);
    Lexer& operator = (const Lexer&);
public:
    Lexer(const char* psz)                          ffThrowAll;
    
    void    Next()                                  ffThrowAll;
    bool    Match(Token tk)                         ffThrowAll;
    Token   Lookup()                          const throw();
    
    void    Error(const char* pszText)        const ffThrowAll;
    void    Expect(Token tk, const char* pszText)   ffThrowAll;
    size_t  ExpectNum(const char* pszText)          ffThrowAll;
    
private:
    void    RewindEmpty()                           ffThrowAll;

    const char*const    m_pBegin;
    const char*         m_pNext;
    mutable size_t      m_nAttr;

    class TokenMap
    {
    public:
        TokenMap() throw();         
        Token operator[] (char ch) const throw() {return m_arr[uns1_t(ch)];}

    private:    
        struct Pair
        {
            char    ch;
            Token   tk;
        };
        static const Pair s_arrPairs[];
        
        Token m_arr[SizeUns1];
    };
    
    static const TokenMap s_map;    
};

}//namespace Search
}//namespace FastFish
#include "Search/Query/Lexer.inl"
