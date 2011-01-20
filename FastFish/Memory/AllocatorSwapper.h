#pragma once
#include "Memory/AllocatorInvader.h"
#include "Common/Common.h"

namespace FastFish{

class AllocatorSwapper:
    public AllocatorInvader
{
    void* Alloc(size_t);
    
    AllocatorSwapper(const AllocatorSwapper&);
    AllocatorSwapper& operator = (const AllocatorSwapper&);
public:
    AllocatorSwapper(size_t nBlockSize, size_t nItemSize, size_t nBlockAlign)   throw() : AllocatorInvader(nBlockSize, nItemSize, nBlockAlign) {}
    ~AllocatorSwapper()                                                         throw() {}

    void* Alloc()                                                               throw();
    void  Free(void* p)                                                         throw();
    void  Clear()                                                               throw();
    
    size_t Count()                                                        const throw();
    
private:
    typedef std::vector<void*> VectorItems;   
    
    VectorItems  m_vecFreeItems;
};    

}//namespace FastFish

#include "Memory/AllocatorSwapper.inl"
