namespace FastFish{

inline
void AllocatorInvaderBB::ClearBB() throw()
{
    for (ListBlocks::iterator it = m_lstBigBlocks.begin(); it != m_lstBigBlocks.end(); ++it)
    {
        delete [] *it;
    }                           
    m_lstBigBlocks.clear();
}

inline
void AllocatorInvaderBB::Check(void* p) const throw()
{
    ffAssume(p);
    ffAssert(m_vecSizes.size() <= m_lstBigBlocks.size());
#ifdef ffDebugDeepChecks
    byte_t* pByte = static_cast<byte_t*>(p);

    ListBlocks::const_iterator itBlock = m_lstBigBlocks.begin();
    VectorSizes::const_iterator itSize = m_vecSizes.begin(); 
    for (;itSize != m_vecSizes.end(); ++itSize, ++itBlock)    
    {
        if (*itBlock <= pByte && pByte < (*itBlock + *itSize))
            break;
    }            
    if (itSize == m_vecSizes.end())
    {
        AllocatorInvader::Check(p);
    }
#endif    
}

inline
void* AllocatorInvaderBB::Alloc(size_t nCount) throw()
{
    ffAssume(m_pDataNext <= m_pDataEnd);    
    ffAssume(nCount > 0);
    
    m_nCount += nCount;
    
    if (c_nMaxSmallBlockSize < nCount)
    {
        m_lstBigBlocks.push_back(new byte_t[nCount + c_nBlockAlign - 1]);
        ffDebugOnly(m_vecSizes.push_back(nCount + c_nBlockAlign - 1));
        byte_t* p    = m_lstBigBlocks.back();
        size_t nMask = c_nBlockAlign - 1;
        size_t nPtr  = size_t(p);
        size_t nAdd  = (c_nBlockAlign - (nPtr & nMask)) & nMask;
        ffAssume(nAdd < c_nBlockAlign);        
        return p + nAdd;
    }
    else
    {
        if (size_t(m_pDataEnd - m_pDataNext) < nCount)
        {
            NewBlock();
        }
        
        void* p = m_pDataNext;
        m_pDataNext += nCount;    
        return p;
    }   
    
}

inline
void AllocatorInvaderBB::Clear() throw()
{
    AllocatorInvader::Clear();
    ClearBB();
}


}//namespace FastFish
