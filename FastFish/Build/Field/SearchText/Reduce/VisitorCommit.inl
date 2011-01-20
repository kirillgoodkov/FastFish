namespace FastFish{
namespace Build{
namespace Reduce{

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
VisitorCommit<bCTRL, bSENS, tit>::VisitorCommit(char* pBuffer, Field* pFld, rank_t rank) throw():
    m_pStrBegin(pBuffer),
    m_pStr(pBuffer),
    m_pField(pFld),
    m_rank(rank)
{
}

template<bool bCTRL, bool bSENS, TextIndexType tit>
void VisitorCommit<bCTRL, bSENS, tit>::operator () (char ch, const NodeT* pNode) throw()
{
    *m_pStr++ = ch;
    memcpy(m_pStr, pNode->Suffix(), pNode->SuffixLen());
    
    if (pNode->Childs().IsEmpty())
    {
        m_pField->CommitWord(m_rank, m_pStrBegin, wordsize_t(m_pStr - m_pStrBegin) + pNode->SuffixLen());
    }
    else
    {
        m_pStr += pNode->SuffixLen();
        pNode->Childs().Enum(*this);
        m_pStr -= pNode->SuffixLen();
    }    
    --m_pStr;
}

}//namespace Reduce
}//namespace Build
}//namespace FastFish
