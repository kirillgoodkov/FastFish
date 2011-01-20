#include "Search/Field/SearchText/sSuffixTree.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Strings.h"
#include "Tools/Bits.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

template<bool bEXACT, bool bEMPTYCHECK>
SuffixTree<bEXACT, bEMPTYCHECK>::SuffixTree(IFStream& ifs) throw()
{
    ifs >> m_nNodesCount
        >> m_nRootCount;

    m_nSizeOfNode = Value2SizeOf(m_nNodesCount);
    m_pTree       = ifs.GetBag<byte_t>(liSearchTree);
    ifs >> m_nSizeOfSuffix;        
    m_nSizeOf     = m_nSizeOfSuffix + m_nSizeOfNode;
    m_pSuffixs    = ifs.GetBag<byte_t>(liSearchSfx);    
}

template<bool bEXACT, bool bEMPTYCHECK>
size_t SuffixTree<bEXACT, bEMPTYCHECK>::FindWord(const char* _pWord, size_t nLen) const throw()
{
    ffAssume(0 < nLen);
    const uns1_t* pWord = reinterpret_cast<const uns1_t*>(_pWord);
    
    size_t nChildCount  = m_nRootCount;
    size_t nChildPos    = 0;
    const byte_t* pNode = m_pTree;
    bool bEmptySet      = true;
    
    while (nLen)
    {
        const byte_t* pRaw = pNode;
        if (FC::TreeBmpStartFrom <= nChildCount)
        {
            const uns1_t* arrBmp = reinterpret_cast<const uns1_t*>(pRaw);
            pRaw += SizeUns1/BitsInByte;
            
            if (!GetBit(arrBmp, *pWord))
                return InvalidPos;

            const uns1_t* arrAccum = reinterpret_cast<const uns1_t*>(pRaw);
            pRaw += SizeUns1/BitsInByte - 1;

            uns1_t nPosHigh = *pWord >> 3;
            uns1_t nMaskLow = 1 << (*pWord & 7);
            nChildPos = g_arrTableCB[arrBmp[nPosHigh] & (nMaskLow - 1)] + 
                        ((0 == nPosHigh) ? 0 : arrAccum[nPosHigh - 1]);
        }            
        else
        {
            const uns1_t* pBegin = reinterpret_cast<const uns1_t*>(pRaw);
            pRaw += nChildCount;
            const uns1_t* pEnd   = reinterpret_cast<const uns1_t*>(pRaw);

            //it's properly, but in practice slower (for Core2, at least)
            /*
            const uns1_t* pChar  = lower_bound(pBegin, pEnd, *pWord);
            if (pChar == pEnd || *pChar != *pWord)
                return InvalidPos;
            */
            
            const uns1_t* pChar = find_if(pBegin, pEnd, bind2nd(equal_to<uns1_t>(), *pWord));
            if (pChar == pEnd)
                return InvalidPos;
                
            nChildPos = pChar - pBegin;
        }
        
        ++pWord; --nLen;
        pRaw += (1 + m_nSizeOf) * nChildPos;      
        
        uns1_t nVal0  = *reinterpret_cast<const uns1_t*>(pRaw++);
        if (Static(bEMPTYCHECK))
            bEmptySet = (0 != (nVal0 & FC::TreeEmptySetFlag));        

        if (!Static(bEXACT) && 0 == nLen) //redundant, added for optimization
            break;           

        bool bInPlace = (0 != (nVal0 & FC::TreeSfxInPlaceFlag));
        nChildCount   = nVal0 & FC::TreeCountMask;
        
        size_t nSuffixSpace = nChildCount ? m_nSizeOfSuffix : m_nSizeOf;
        const char* pSuffix = bInPlace ?
                                reinterpret_cast<const char*>(pRaw) :
                                (m_pSuffixs + ReadVal<size_t>(pRaw, m_nSizeOfSuffix));
        pRaw += nSuffixSpace;
        
        size_t nSuffixLen = bInPlace ? StrLenN(pSuffix, nSuffixSpace) : strlen(pSuffix);
                                
        if (0 == nChildCount)
        {
            if (Static(bEXACT) && nLen < nSuffixLen) 
                return InvalidPos;
            
            if (nSuffixLen < nLen || nLen != EqualCount(pSuffix, pWord, nLen))
                return InvalidPos;
                
            break;                                    
        }
        else
        {
            if (nSuffixLen < nLen)
            {
                if (nSuffixLen != EqualCount(pSuffix, pWord, nSuffixLen))
                    return InvalidPos;
                
                pWord += nSuffixLen; 
                nLen  -= nSuffixLen;
                pNode  = m_pTree + (2 + m_nSizeOf)*ReadVal<size_t>(pRaw, m_nSizeOfNode);
                continue;                
            }
            else
            {
                if (nLen != EqualCount(pSuffix, pWord, nLen))
                    return InvalidPos;
                                    
                break;
            }
        }
    }
    
    if (Static(bEMPTYCHECK) && bEmptySet)
    {
        return InvalidPos;
    }
    
    return (pNode - m_pTree)/(2 + m_nSizeOf) + nChildPos;
}

template class SuffixTree<false, false>;
template class SuffixTree<false, true>;
template class SuffixTree<true,  true>;

}//namespace Search
}//namespace FastFish
