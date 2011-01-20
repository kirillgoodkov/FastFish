#pragma once
#include "Common/Common.h"

namespace FastFish{

class IFStream
{
    IFStream(const IFStream&);
    IFStream& operator = (const IFStream&);
public:
    IFStream(const MemMap& mmap, MemManager& mman)          throw();
    
    void Read(void* p, size_t nSize)                        throw();
    const void* GetBagRaw(size_t nLayer, size_t nAlign = 1) throw();    

    template<typename TYPE>
    IFStream& operator >> (TYPE& val)                       throw();
    
    template<typename TYPE>
    const TYPE* GetBag(size_t nLayer, size_t nAlign = 1)    throw()     {return static_cast<const TYPE*>(GetBagRaw(nLayer, nAlign));}

private:
    const MemMap&       c_mmap;   
    MemManager&         m_mman;

    const byte_t*       m_pFront;
    const byte_t*       m_pBack;
    
};


}//namespace FastFish
#include "Tools/IFStream.inl"
