#include "Common/Templates.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit>
Splitter<BASE, bCTRL, bSENS, tit>::Splitter(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw():
    BASE(tplIndex, tpl, pDocs, a),
    c_cspn(tpl.pszDelimiters, !bCTRL),
    c_nMaxLen(tpl.nWordLenMax)
{
    ffAssumeUser(0 < tpl.nWordLenMin, "invalid template");
    MapTypePd& mapSkip = const_cast<MapTypePd&>(c_mapSkip);
    for (size_t n = 0; n < BASE::c_tpl.nStopWordsCount; ++n)
    {
        const char* pszWord = BASE::c_tpl.arrStopWords[n];
        size_t nLen = strlen(pszWord);
        ffAssumeUser(pszWord && 0 < nLen && nLen <= MaxWordSize, "invalid template");
        
        if (Static(bSENS))
        {
            nLen = UTF8Check(pszWord, nLen);
        }
        else
        {
            char* pBuff = static_cast<char*>(BASE::m_a.aSkipStr.Alloc(nLen));
            nLen        = wordsize_t(UTF8ToLower(pszWord, nLen, pBuff));
            pszWord     = pBuff;            
        }
        
        mapSkip.val.AddOrGet(*pszWord, BASE::m_a.aSkipMap).AddWord(pszWord + 1, wordsize_t(nLen - 1), BASE::m_a.aSkipMap);
    }
}
    
template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit>
void Splitter<BASE, bCTRL, bSENS, tit>::Add(const void* pData, datasize_t nSize) throw()
{
    ffAssumeUser(0 == nSize, "invalid argument nSize");
    const char* pszText = static_cast<const char*>(pData);
    
    while (*pszText)
    {
        size_t nLenFull = c_cspn(pszText);
        
        if (0 != nLenFull)    
        {
            wordsize_t nLen = wordsize_t(std::min(nLenFull, c_nMaxLen));            
            
            if (ffIsLogged(lsWarnWordCut) && c_nMaxLen < nLenFull)
            {
                ffLog(lsWarnWordCut, "word \"%.20s...\" restricted to %u symbols\n", pszText, c_nMaxLen);
            }                
            
            if (Static(bSENS))
            {
                nLen = wordsize_t(UTF8Check(pszText, nLen));
            }                          
                        
            char* pWord = static_cast<char*>(BASE::m_a.aRStr.Alloc(nLen));
            
            if (Static(bSENS))
            {
                memcpy(pWord, pszText, nLen);
            }
            else
            {
                nLen = wordsize_t(UTF8ToLower(pszText, nLen, pWord));
            }
            
            if (nLen && !NodeType::Find(c_mapSkip.val, pWord, nLen))
            {
                if (Static(titInfix == tit))
                {
                    while (UTF8LongerThan(pWord, nLen, BASE::c_tpl.nWordLenMin - 1))
                    {
                        BASE::AddWord(pWord, nLen);
                        uns1_t nSymbolLen = UTF8SymbolLen(*pWord);
                        ffAssume(0 != nSymbolLen);
                        pWord += nSymbolLen;
                        nLen   = nLen - nSymbolLen;
                    }
                }
                else
                {
                    if (UTF8LongerThan(pWord, nLen, BASE::c_tpl.nWordLenMin - 1))
                    {
                        BASE::AddWord(pWord, nLen);
                    }
                }
            }
            
            pszText += nLenFull;
        }                
        
        if (0 != *pszText)
        {
            ++pszText;
        }            
    }       
    
}

}//namespace Reduce
}//namespace Build
}//namespace FastFish
