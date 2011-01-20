#pragma once
#include "Common/Common.h"

namespace FastFish{

template<typename VALUE, typename ALLOCATOR, size_t BlockShift>
class SList
{
    SList(const SList&);
    SList& operator = (const SList&);
public:
    SList()                                throw();
    
    void PushBack(VALUE val, ALLOCATOR& a) throw();
    void PopBack(ALLOCATOR& a)             throw();
    void Append(SList& lst)                throw();    
    
    void Clear()                           throw();
    
    size_t        Count()            const throw()      {return m_nCount;}
    const VALUE&  Back()             const throw();

    template<typename PROC> 
    void Enum(PROC& proc)            const ffThrowAll;
    
    static const size_t BlockSize  = 1 << BlockShift;
    static const size_t BlockMask  = BlockSize - 1;
    static const size_t ItemsCount = (BlockSize - sizeof(void*))/sizeof(VALUE);    
private:

    struct Block
    {
        VALUE*  pPrev;
        VALUE   arrVal[ItemsCount];
    };

    size_t  m_nCount;
    VALUE*  m_pLast;
    Block*  m_pFirstBlock;               
};

    


}//namespace FastFish
#include "Containers/SList.inl"
