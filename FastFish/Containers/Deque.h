#pragma once
#include "Common/Common.h"

namespace FastFish{

template<typename VALUE, size_t BlockShift>
class Deque 
{
    Deque(const Deque&);
    Deque& operator = (const Deque&);
public:
    Deque()                                 throw()     : m_nCount(0){}
    ~Deque()                                throw()     {Clear();}
    
    size_t PushBack(const VALUE& src)       throw();
    void   PopBack()                        throw();
    void   Clear()                          throw();
    
    VALUE& Back()                           throw();
    const VALUE& Back()               const throw();
    
    VALUE& operator[](size_t n)             throw();    
    const VALUE& operator[](size_t n) const throw();    
    size_t Count()                    const throw()     {return m_nCount;}

    void Store(OFStream& file)        const ffThrowAll;

    typedef VALUE ValueType;
private:
    typedef std::vector<VALUE*> VectorBlocks;
    static const size_t BlockSize = 1 << BlockShift;
    static const size_t BlockMask = BlockSize - 1;

    VectorBlocks m_vecBlocks;
    size_t       m_nCount;
};

template<typename VALUE, size_t BlockShift>
OFStream& operator << (OFStream& file, const Deque<VALUE, BlockShift>& deq) ffThrowAll;

}//namespace FastFish

#include "Containers/Deque.inl"

