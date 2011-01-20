#pragma once
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{

class DequeRaw
{
    DequeRaw(const DequeRaw&);
    DequeRaw& operator = (const DequeRaw&);
public:
    DequeRaw(size_t nItemSize)              throw();
    ~DequeRaw()                             throw()         {Clear();}
    
    size_t PushBack(const void* pSrc)       throw();
    void   PopBack()                        throw();
    void   Clear()                          throw();
    
    const void* operator[](size_t n)  const throw();    
    size_t Count()                    const throw()         {return m_nCount;}
    size_t ItemSize()                 const throw()         {return c_nItemSize;}
    
private:    
    static const size_t BlockShift = PC::DequeRawShift;
    static const size_t BlockSize  = 1 << BlockShift;
    static const size_t BlockMask  = BlockSize - 1;
    
    const size_t            c_nItemSize;
    
    std::vector<byte_t*>    m_vecBlocks;
    size_t                  m_nCount;
};


}//namespace FastFish

#include "Containers/DequeRaw.inl"

