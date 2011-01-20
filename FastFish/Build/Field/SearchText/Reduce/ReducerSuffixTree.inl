#include "Build/Field/SearchText/Reduce/VisitorCommit.h"
namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit> 
ReducerSuffixTree<BASE, bCTRL, bSENS, tit>::ReducerSuffixTree(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw():
    BASE(tplIndex, tpl, pDocs, a),
    m_vecStrTemp(1)
{
}


template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit>
void ReducerSuffixTree<BASE, bCTRL, bSENS, tit>::Commit(rank_t rank) throw()
{
    VisitorCommit<bCTRL, bSENS, tit> visWordsCommit(&m_vecStrTemp[0], this, rank);
    m_mapRoot.val.Enum(visWordsCommit);

    MapTypePd newMap;
    m_mapRoot.val = newMap.val;
}

template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit> 
void ReducerSuffixTree<BASE, bCTRL, bSENS, tit>::AddWord(const char* pWord, wordsize_t nLen) throw()
{
    m_mapRoot.val.AddOrGet(*pWord, BASE::m_a.aRMap).AddWord(pWord + 1, nLen - 1, BASE::m_a.aRMap);
    if (m_vecStrTemp.capacity() < nLen)
    {
        m_vecStrTemp.reserve(nLen);
    }
}


}//namespace Reduce
}//namespace Build
}//namespace FastFish
