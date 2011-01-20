#include "Build/Field/SearchText/bNode.h"
#include "Tools/OFStream.h"
#include "Common/Consts.h"

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit> 
ReorderBFS<bCTRL, bSENS, tit>::ReorderBFS(size_t nCount) throw():
    c_nCount(nCount),
    m_arrNodes(nCount),
    m_arrChars(nCount),
    m_nNextPos(0)        
{
}

template<bool bCTRL, bool bSENS, TextIndexType tit>
void ReorderBFS<bCTRL, bSENS, tit>::Reorder(MapType& mapRoot) throw()
{
    mapRoot.Enum(*this);

    for (size_t n = 0; n < m_nNextPos; ++n)
    {
        m_arrNodes[n]->m_mapChilds.Enum(*this);
    }
    
    ffAssume(c_nCount == m_nNextPos);
}

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
const Node<bCTRL, bSENS, tit>& ReorderBFS<bCTRL, bSENS, tit>::At(size_t n) const throw()
{
    ffAssume(c_nCount == m_nNextPos);
    ffAssume(n < c_nCount);
    
    return *m_arrNodes[n];
}

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
void ReorderBFS<bCTRL, bSENS, tit>::operator()(char ch, NodeType*& pNode) throw()
{
    m_arrNodes[m_nNextPos] = pNode;
    m_arrChars[m_nNextPos] = ch;
    
    pNode = reinterpret_cast<NodeType*>(m_nNextPos);
    ++m_nNextPos;            
}

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
void ReorderBFS<bCTRL, bSENS, tit>::StoreCharsFrom(OFStream& file, size_t nPos, size_t nCount) const ffThrowAll
{
    if (FC::TreeBmpStartFrom <= nCount)
    {
        uns1_t arrBmp[SizeUns1/BitsInByte] = {0};                
        for (size_t n = nPos; n < nCount + nPos; ++n)
        {
            uns1_t ch = m_arrChars[n];
            SetBit(arrBmp, ch);
        }
        
        uns1_t arrAccum[SizeUns1/BitsInByte - 1] = {g_arrTableCB[arrBmp[0]], 0};
        for (size_t n = 1; n < ffCountOf(arrAccum); ++n)
        {
            arrAccum[n] = arrAccum[n - 1] + g_arrTableCB[arrBmp[n]];
        }
        
        ffAssumeStatic(sizeof(arrBmp) + sizeof(arrAccum) <= FC::TreeBmpStartFrom);
        file.Write(arrBmp, sizeof(arrBmp));
        file.Write(arrAccum, sizeof(arrAccum));
    }
    else
    {
        if (0 != nCount)
        {
            file.Write(&m_arrChars[nPos], nCount);
        }
    }                       
}

}//namespace Build
}//namespace FastFish
