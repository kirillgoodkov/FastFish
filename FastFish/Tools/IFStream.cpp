#include "Tools/IFStream.h"
#include "Tools/MemMap.h"
#include "Tools/MemManager.h"
#include "Tools/Bits.h"

namespace FastFish{

IFStream::IFStream(const MemMap& mmap, MemManager& mman) throw():
    c_mmap(mmap),
    m_mman(mman),
    m_pFront(static_cast<const byte_t*>(c_mmap.pData)),
    m_pBack(m_pFront + c_mmap.nSize)
{
    m_mman.AddSegment();
}

const void* IFStream::GetBagRaw(size_t nLayer, size_t nAlign) throw()
{
    filepos_t nSize = 0;
    *this >> nSize;
    const byte_t* pData = m_pFront;
    m_pFront += nSize;

    ffAssume(nAlign <= SizeUns1);
    ffAssert(MinGreateOrEqualPow2(nAlign) == nAlign);
    
    size_t nMask = nAlign - 1;
    size_t nAdd  = (nAlign - (size_t(pData) & nMask)) & nMask;
    ffAssume(nAdd < nAlign);

    m_mman.AddBag(nLayer, pData, size_t(nSize));
    return pData + nAdd;
}

}//namespace FastFish
