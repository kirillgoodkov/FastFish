namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS>
ReducerNop<BASE, bCTRL, bSENS>::ReducerNop(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw():
    BASE(tplIndex, tpl, pDocs, a)
{
}

template<typename BASE, bool bCTRL, bool bSENS>
void ReducerNop<BASE, bCTRL, bSENS>::Commit(rank_t rank) throw()
{
    for (typename std::vector<Word>::const_iterator it = m_vecWords.begin(); it != m_vecWords.end(); ++it)
    {
        BASE::CommitWord(rank, it->pStr, it->nLen);
    }
    m_vecWords.clear();
}

template<typename BASE, bool bCTRL, bool bSENS>
void ReducerNop<BASE, bCTRL, bSENS>::AddWord(const char* pWord, wordsize_t nLen) throw()
{
    char* p = static_cast<char*>(BASE::m_a.aRStr.Alloc(nLen));
    memcpy(p, pWord, nLen);
    Word w = {p, nLen};
    m_vecWords.push_back(w);
}



}//namespace Reduce
}//namespace Build
}//namespace FastFish
