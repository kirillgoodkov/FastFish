#pragma once
#include "Memory/AllocatorInvader.h"
#include "Common/Common.h"

namespace FastFish{

//BB - Big Block

class AllocatorInvaderBB:
    private AllocatorInvader
{
    AllocatorInvaderBB(const AllocatorInvaderBB&);
    AllocatorInvaderBB& operator = (const AllocatorInvaderBB&);
public:
    AllocatorInvaderBB(size_t nBlockSize, size_t nBlockAlign = 1) throw();
    ~AllocatorInvaderBB()                                         throw()       {ClearBB();}
    
    void* Alloc(size_t nCount)                                    throw();
    void  Clear()                                                 throw();
    
    void  Check(void* p)                                    const throw();    
    size_t Count()                                          const throw()       {return m_nCount;}
    
protected:
    void ClearBB() throw();

    typedef std::vector<size_t> VectorSizes;
    
    static const size_t SmallBlockShift = 2;
    
    const size_t    c_nMaxSmallBlockSize;

    ListBlocks      m_lstBigBlocks;
    size_t          m_nCount;
#ifdef ffDebug
    VectorSizes     m_vecSizes;
#endif    
};    

}//namespace FastFish

#include "Memory/AllocatorInvaderBB.inl"
