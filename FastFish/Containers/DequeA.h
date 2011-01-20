#pragma once
#include "Common/Common.h"

namespace FastFish{

template<typename VALUE, size_t BlockShift>
class DequeA //with allocator support
{
    DequeA(const DequeA&);
    DequeA& operator = (const DequeA&);
public:
    DequeA()                                               throw()  : m_nCount(0) {}
    
    size_t PushBack(const VALUE& src, AllocatorInvader& a) throw();
    
    const VALUE& operator[](size_t n)                const throw();    
    size_t Count()                                   const throw()  {return m_nCount;}

private:
    typedef std::vector<VALUE*> VectorBlocks;
    static const size_t BlockSize = 1 << BlockShift;
    static const size_t BlockMask = BlockSize - 1;

    VectorBlocks m_vecBlocks;
    size_t       m_nCount;
};

}//namespace FastFish

#include "Containers/DequeA.inl"

