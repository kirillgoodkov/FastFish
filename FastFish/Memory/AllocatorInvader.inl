namespace FastFish{

inline
void AllocatorInvader::CheckAlign(void* p) const  throw()
{
    ffAssume(0 == (size_t(p) % c_nBlockAlign));
}

inline
void AllocatorInvader::Check(void* p) const  throw()
{
    CheckAlign(p);    

#ifdef ffDebugDeepChecks
    byte_t* pByte = static_cast<byte_t*>(p);
    ListBlocks::const_iterator it = m_lstBlocks.begin();
    for (;it != m_lstBlocks.end(); ++it)    
        if (*it <= pByte && pByte < (*it + c_nBlockSize))
            break;
    ffAssert(it != m_lstBlocks.end());
#endif
}

inline
void AllocatorInvader::InitPData() throw()
{
    ffAssert(!m_lstBlocks.empty());
    m_pDataNext = m_lstBlocks.back();
    
    size_t nMask = c_nBlockAlign - 1;
    size_t nPtr  = size_t(m_pDataNext);
    size_t nAdd  = (c_nBlockAlign - (nPtr & nMask)) & nMask;
    ffAssume(nAdd < c_nBlockAlign);
    m_pDataNext += nAdd;
    m_pDataEnd   = m_pDataNext + c_nBlockSize;
}


inline
void* AllocatorInvader::Alloc() throw()
{
    ffAssume(m_pDataNext <= m_pDataEnd);    
    
    if (m_pDataEnd == m_pDataNext)
    {
        NewBlock();
    }
    
    void* p = m_pDataNext;
    m_pDataNext += c_nItemSize;    
    return p;
};


inline
void* AllocatorInvader::Alloc(size_t nCount) throw()
{
    ffAssume(m_pDataNext <= m_pDataEnd);    
    ffAssume(nCount * c_nItemSize <= c_nBlockSize);
    
    if (size_t(m_pDataEnd - m_pDataNext) < nCount * c_nItemSize)
    {
        NewBlock();
    }
    
    void* p = m_pDataNext;
    m_pDataNext += nCount * c_nItemSize;    
    return p;
};

inline
void AllocatorInvader::Clear() throw()
{
    if (m_lstBlocks.size())
    {   
        while (1 < m_lstBlocks.size())
        {
            delete [] m_lstBlocks.front();
            m_lstBlocks.pop_front();
        }
        
        InitPData();                
    }
}

template<typename VALUE, typename ALLOCTYPE> 
VALUE* New(ALLOCTYPE& a) throw()
{
    ffAssert(sizeof(VALUE) == a.ItemSize());
    
    VALUE* p = static_cast<VALUE*>(a.Alloc());
    new (p) VALUE();
    return p;
}

template<typename VALUE, typename ALLOCTYPE> 
VALUE* NewPOD(ALLOCTYPE& a) throw()
{
    ffAssert(sizeof(VALUE) == a.ItemSize());
    VALUE* p = static_cast<VALUE*>(a.Alloc());
    return p;
}

template<typename VALUE, typename ALLOCTYPE> 
VALUE* NewPODUnkSize(ALLOCTYPE& a) throw()
{
    VALUE* p = static_cast<VALUE*>(a.Alloc());
    return p;
}

template<typename VALUE, typename ALLOCTYPE> 
VALUE* NewFromRaw(ALLOCTYPE& a) throw()
{
    VALUE* p = static_cast<VALUE*>(a.Alloc(sizeof(VALUE)));
    new (p) VALUE();
    return p;
}

template<typename VALUE, typename ALLOCTYPE> 
VALUE* NewPODFromRaw(ALLOCTYPE& a) throw()
{
    VALUE* p = static_cast<VALUE*>(a.Alloc(sizeof(VALUE)));
    return p;
}


}//namespace FastFish
