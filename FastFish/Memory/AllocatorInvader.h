#pragma once
#include "Common/Common.h"

namespace FastFish{

class AllocatorInvader
{
    AllocatorInvader& operator = (const AllocatorInvader&);
    AllocatorInvader(const AllocatorInvader& src);
public:
    AllocatorInvader(size_t nBlockSize, size_t nItemSize, size_t nBlockAlign) throw();
    ~AllocatorInvader()                                                       throw();
    
    void* Alloc()                                                             throw();
    void* Alloc(size_t)                                                       throw();
    void  Free(void*)                                                         throw()       {};  
    void  Clear()                                                             throw();
    
    void   Check(void* p)                                               const throw();
    size_t ItemSize()                                                   const throw()       {return c_nItemSize;}

protected:
    typedef std::list<byte_t*> ListBlocks;   

    void CheckAlign(void* p) const throw();
    void InitPData() throw();
    
    virtual void NewBlock() throw();          

    const size_t    c_nBlockSize;
    const size_t    c_nItemSize;
    const size_t    c_nBlockAlign;
    
    ListBlocks      m_lstBlocks;
    byte_t*         m_pDataNext;
    const byte_t*   m_pDataEnd;
};    

template<typename VALUE, typename ALLOCTYPE>
VALUE* New(ALLOCTYPE& a) throw();

template<typename VALUE, typename ALLOCTYPE>
VALUE* NewPOD(ALLOCTYPE& a) throw();

template<typename VALUE, typename ALLOCTYPE>
VALUE* NewPODUnkSize(ALLOCTYPE& a) throw();

template<typename VALUE, typename ALLOCTYPE>
VALUE* NewFromRaw(ALLOCTYPE& a) throw();

template<typename VALUE, typename ALLOCTYPE>
VALUE* NewPODFromRaw(ALLOCTYPE& a) throw();


}//namespace FastFish

#include "Memory/AllocatorInvader.inl"
