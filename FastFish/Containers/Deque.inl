#include "Tools/OFStream.h"

namespace FastFish{

template<typename VALUE, size_t BlockShift> 
size_t Deque<VALUE, BlockShift>::PushBack(const VALUE& src) throw()
{
    size_t nPosition = m_nCount&BlockMask;
    if (0 == nPosition)
    {
        m_vecBlocks.push_back(new VALUE[BlockSize]);
    }
    new (m_vecBlocks.back() + nPosition) VALUE(src);
    return m_nCount++;
}

template<typename VALUE, size_t BlockShift> 
void Deque<VALUE, BlockShift>::PopBack() throw()
{   
    --m_nCount;    
    size_t nPosition = m_nCount&BlockMask;    
    (m_vecBlocks.back() + nPosition)->~VALUE();
    if (0 == nPosition)
    {
        delete [] m_vecBlocks.back();
        m_vecBlocks.pop_back();
    }               
}

template<typename VALUE, size_t BlockShift> 
void Deque<VALUE, BlockShift>::Clear() throw()
{
    for (typename VectorBlocks::iterator it = m_vecBlocks.begin();
         it != m_vecBlocks.end();
         ++it)
    {
        ffAssert(*it);
        delete [] *it;
    }             
    m_vecBlocks.clear();
    m_nCount = 0;
}

template<typename VALUE, size_t BlockShift> 
VALUE& Deque<VALUE, BlockShift>::Back() throw()
{
    size_t nPosition = (m_nCount - 1) & BlockMask;    
    return m_vecBlocks.back()[nPosition];    
}

template<typename VALUE, size_t BlockShift> 
const VALUE& Deque<VALUE, BlockShift>::Back() const throw()
{
    size_t nPosition = (m_nCount - 1) & BlockMask;
    return m_vecBlocks.back()[nPosition];    
}

template<typename VALUE, size_t BlockShift> 
VALUE& Deque<VALUE, BlockShift>::operator[](size_t n) throw()
{
    ffAssume(n < m_nCount);
    return m_vecBlocks[n>>BlockShift][n&BlockMask];
}

template<typename VALUE, size_t BlockShift> 
const VALUE& Deque<VALUE, BlockShift>::operator[](size_t n) const throw()
{
    ffAssume(n < m_nCount);
    return m_vecBlocks[n>>BlockShift][n&BlockMask];
}

template<typename VALUE, size_t BlockShift>
void Deque<VALUE, BlockShift>::Store(OFStream& file) const ffThrowAll
{
    size_t n = m_nCount;
    size_t nBlockSize = BlockSize;
    for (typename VectorBlocks::const_iterator it = m_vecBlocks.begin();
         it != m_vecBlocks.end();
         ++it, n -= BlockSize)
    {
        file.Write(*it, sizeof(VALUE) * std::min(nBlockSize, n));
    }   
}

template<typename VALUE, size_t BlockShift> 
OFStream& operator << (OFStream& file, const Deque<VALUE, BlockShift>& deq) ffThrowAll
{
    deq.Store(file);
    return file;
}


}//namespace FastFish

