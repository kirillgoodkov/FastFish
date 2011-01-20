#include "Memory/AllocatorInvader.h"

namespace FastFish{

template<typename VALUE, size_t BlockShift> 
size_t DequeA<VALUE, BlockShift>::PushBack(const VALUE& src, AllocatorInvader& a) throw()
{
    size_t nPosition = m_nCount&BlockMask;
    if (0 == nPosition)
    {
        ffAssert(a.ItemSize() == sizeof(VALUE) * BlockSize);
        m_vecBlocks.push_back(static_cast<VALUE*>(a.Alloc()));
    }
    new (m_vecBlocks.back() + nPosition) VALUE(src);
    return m_nCount++;    
}

template<typename VALUE, size_t BlockShift> 
const VALUE& DequeA<VALUE, BlockShift>::operator[](size_t n) const throw()
{
    ffAssume(n < m_nCount);
    return m_vecBlocks[n>>BlockShift][n&BlockMask];
}

}//namespace FastFish

