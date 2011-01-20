namespace FastFish{

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
void SList<VALUE, ALLOCATOR, BlockShift>::Clear() throw()
{
    m_nCount      = 0;
    m_pLast       = 0;
    m_pFirstBlock = 0;    
}

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
SList<VALUE, ALLOCATOR, BlockShift>::SList() throw()
{
    Clear();
    ffAssumeStatic(0 == sizeof(void*)%sizeof(VALUE));
}

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> ffForceInline
void SList<VALUE, ALLOCATOR, BlockShift>::PushBack(VALUE val, ALLOCATOR& a) throw()
{
    ffAssert(a.ItemSize() == BlockSize);

    if (0 == (reinterpret_cast<size_t&>(m_pLast) & BlockMask))
    {
        Block* pNewBlock = NewPOD<Block>(a);
        ffAssume(0 == (reinterpret_cast<size_t&>(pNewBlock) & BlockMask));
        
        pNewBlock->pPrev = m_pLast;
        if (0 == pNewBlock->pPrev)
        {
            m_pFirstBlock = pNewBlock;
        }
        else
        {
            ffAssume(m_pFirstBlock);
        }
        m_pLast = pNewBlock->arrVal;
    }
    
    *m_pLast++ = val;
    ++m_nCount;    
}

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
void SList<VALUE, ALLOCATOR, BlockShift>::PopBack(ALLOCATOR& a) throw()
{
    ffAssume(m_nCount);
    
    --m_pLast;
    --m_nCount;
    Block* pBlock = reinterpret_cast<Block*>(reinterpret_cast<size_t&>(m_pLast) & ~BlockMask);
    if (m_pLast == pBlock->arrVal)
    {
        m_pLast = pBlock->pPrev;
        if (0 == m_pLast)
        {
            ffAssume(0 == m_nCount);
            m_pFirstBlock = 0;
        }
        a.Free(pBlock);
    }                          
}

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
void SList<VALUE, ALLOCATOR, BlockShift>::Append(SList& lst) throw()
{
    if (0 == lst.m_nCount)
        return;
        
    if (0 == m_nCount)
    {
        m_nCount      = lst.m_nCount;
        m_pLast       = lst.m_pLast;
        m_pFirstBlock = lst.m_pFirstBlock;
    }
    else
    {
        lst.m_pFirstBlock->pPrev = m_pLast;
        m_pLast                  = lst.m_pLast;
        m_nCount                += lst.m_nCount;
    }
    
    lst.Clear();    
}

template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
const VALUE& SList<VALUE, ALLOCATOR, BlockShift>::Back() const throw()
{
    ffAssume(m_pLast);
    ffAssume(m_nCount);
    return *(m_pLast - 1);
}


template<typename VALUE, typename ALLOCATOR, size_t BlockShift> 
template<typename PROC> 
void SList<VALUE, ALLOCATOR, BlockShift>::Enum(PROC& proc) const ffThrowAll
{
    Block* pBlock = reinterpret_cast<Block*>(reinterpret_cast<size_t>(m_pLast - 1) & ~BlockMask);
    ffDebugOnly(size_t nCount = 0);
    for (VALUE* p = m_pLast; p; ffDebugOnly(++nCount))
    {
        proc(*--p);        
        if (p == pBlock->arrVal)
        {
            p = pBlock->pPrev;
            pBlock = reinterpret_cast<Block*>(reinterpret_cast<size_t>(p - 1) & ~BlockMask); 
        }
    }
    ffAssert(m_nCount == nCount);
}


}//namespace FastFish
