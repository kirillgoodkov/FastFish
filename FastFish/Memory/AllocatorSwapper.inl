namespace FastFish{

inline
void* AllocatorSwapper::Alloc() throw()
{
    if (m_vecFreeItems.empty())
    {
        return AllocatorInvader::Alloc();
    }
    else
    {
        void* p = m_vecFreeItems.back();
        m_vecFreeItems.pop_back();        
        return p;                                 
    }
}

inline
void AllocatorSwapper::Free(void* p) throw()
{
    Check(p);    
    m_vecFreeItems.push_back(p);    
}

inline
void AllocatorSwapper::Clear() throw()
{
    AllocatorInvader::Clear();
    m_vecFreeItems.clear();
}

inline
size_t AllocatorSwapper::Count() const throw()
{
    return (m_lstBlocks.size()*c_nBlockSize - (m_pDataEnd - m_pDataNext))/c_nItemSize 
           - m_vecFreeItems.size();
}


}//namespace FastFish
