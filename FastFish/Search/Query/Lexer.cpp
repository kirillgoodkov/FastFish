#include "Search/Query/Lexer.h"

using namespace std;

namespace FastFish{
namespace Search{

const Lexer::TokenMap::Pair Lexer::TokenMap::s_arrPairs[] = 
{
    {'!',tkEopNot},
    {'|',tkEopOr},
    {'&',tkEopAnd},
    {'a',tkSopContainAll},  {'A',tkSopContainAll},
    {'s',tkSopContainSome}, {'S',tkSopContainSome},
    {',',tkComma},
    {'(',tkBrRdOpen},       {')',tkBrRdClose},
    {'{',tkBrCrOpen},       {'}',tkBrCrClose},
    {' ',tkWhiteSpace}              
};


Lexer::TokenMap::TokenMap() throw()
{
    m_arr[0] = tkEnd;
    fill(m_arr + 1,     m_arr + 0x20, tkWhiteSpace);
    fill(m_arr + 0x20,  m_arr + ffCountOf(m_arr), tkUnknown);
    fill(m_arr + '0',   m_arr + '9' + 1, tkNumber);
    for (size_t n = 0; n < ffCountOf(s_arrPairs); ++n)
    {
        m_arr[uns1_t(s_arrPairs[n].ch)] = s_arrPairs[n].tk;
    }
}

const Lexer::TokenMap Lexer::s_map;

Lexer::Lexer(const char* psz) ffThrowAll:
    m_pBegin(psz),
    m_pNext(psz),
    m_nAttr(0)
{
    RewindEmpty();
}

void Lexer::Error(const char* pszText) const ffThrowAll
{
    std::string str(pszText);
    str += ", position: ";
    char arrBuff[11];
    sprintf(arrBuff, "%u", unsigned(m_pNext - m_pBegin));
    str += arrBuff;
    throw ExceptSyntax(str);
}


}//namespace Search
}//namespace FastFish
