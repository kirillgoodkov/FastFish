#include "Build/Field/SearchText/Reduce/brNode.h"
#include "Build/Field/bField.h"
#include "Tools/Bits.h"
                                              
using namespace std;

namespace FastFish{
namespace Build{
namespace Reduce{

//---------------------------------------------------------------------------

template<bool bCTRL, bool bSENS> 
Node<bCTRL, bSENS>::Node() throw():
    m_nSuffixLen(0)
{
    ffAssumeStatic(0 == sizeof(*this)%sizeof(void*));
}

template<bool bCTRL, bool bSENS>
void Node<bCTRL, bSENS>::Split(wordsize_t nFrom, AllocatorInvader& a) throw()
{
    ffAssume(nFrom < m_nSuffixLen);
    MapTypePd mapNew;
    Node& child        = mapNew.val.AddOrGet(m_pSuffix[nFrom], a);
    child.m_nSuffixLen = m_nSuffixLen - nFrom - 1;
    child.m_pSuffix    = m_pSuffix + nFrom + 1;
    child.m_mapChilds  = m_mapChilds;
    m_mapChilds        = mapNew.val;
    m_nSuffixLen       = nFrom;    
}

//---------------------------------------------------------------------------
//recursive

template<bool bCTRL, bool bSENS>
void Node<bCTRL, bSENS>::AddWord(const char* pWord, wordsize_t nLen, AllocatorInvader& a) throw()
{
    if (0 == nLen)
    {
        return;        
    }               

    wordsize_t nEqualCount = EqualCount(m_pSuffix, pWord, min(nLen, m_nSuffixLen));
    
    if (nLen == nEqualCount)
    {
        return; 
    }        
    
    if (nEqualCount < m_nSuffixLen)
    {
        Split(nEqualCount, a);
    }
    
    if (nEqualCount < nLen)//add
    {
        ffAssume(nEqualCount == m_nSuffixLen);
        if (m_mapChilds.IsEmpty())
        {
            m_pSuffix    = pWord;
            m_nSuffixLen = nLen;
        }
        else
        {
            wordsize_t nCut = nEqualCount + 1;
            ffAssume(nCut <= nLen);
            m_mapChilds.AddOrGet(pWord[nEqualCount], a).AddWord(pWord + nCut, nLen - nCut, a);
        }
    }
}

//---------------------------------------------------------------------------

template class Node<false, false>;
template class Node<false, true>;
template class Node<true, false>;
template class Node<true, true>;

}//namespace Reduce
}//namespace Build
}//namespace FastFish
