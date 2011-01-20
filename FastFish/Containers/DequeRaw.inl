namespace FastFish{

inline
DequeRaw::DequeRaw(size_t nItemSize) throw():
    c_nItemSize(nItemSize),
    m_nCount(0)
{
    ffAssume(0 != nItemSize);
}

inline
size_t DequeRaw::PushBack(const void* pSrc) throw()
{
    size_t nRest = m_nCount&BlockMask;
    if (0 == nRest)
    {
        m_vecBlocks.push_back(new byte_t[c_nItemSize * BlockSize]);
    }
    memcpy(m_vecBlocks.back() + nRest*c_nItemSize, pSrc, c_nItemSize);
    return m_nCount++;
}

inline
void DequeRaw::PopBack() throw()
{   
    --m_nCount;    
    size_t nPosition = m_nCount&BlockMask;    
    if (0 == nPosition)
    {
        delete [] m_vecBlocks.back();
        m_vecBlocks.pop_back();
    }               
}

inline
const void* DequeRaw::operator[](size_t n) const throw()
{
    ffAssume(n < m_nCount);
    return &m_vecBlocks[n>>BlockShift][(n&BlockMask) * c_nItemSize];    
}

}//namespace FastFish


