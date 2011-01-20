#include "Build/Documents/bDocuments.h"
#include "Common/Templates.h"

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
void FieldSearchText<bCTRL, bSENS, tit>::CommitWord(rank_t rank, const char* pWord, wordsize_t nLen) throw()
{
#ifdef ffDebug            
    for (size_t n = 0; n < nLen; ++n)
    {
        ffAssert(CharMapUtils::IsValidChar(uns1_t(pWord[n])));
    }                
#endif
    size_t nOldCount = m_a.aMap.aItem.Count();
    typename NodeType::ContextAW ctx = {c_pDocs->Count(), &m_a, pWord + 1, nLen - 1, uns4_t(UTF8SymbolPos(pWord, pWord + nLen, c_tpl.nWordLenMin - 1))};
    ffAssert(ctx.nSkipLen < nLen);    
    m_arrRanks[rank].val.AddOrGet(*pWord, m_a.aMap).AddWord(ctx);
    ffAssert(nOldCount <= m_a.aMap.aItem.Count());
    m_nNodesCount += m_a.aMap.aItem.Count() - nOldCount;
}

}//namespace Build
}//namespace FastFish
